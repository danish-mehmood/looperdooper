#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "eventloop.h"
#include "platform.h"
#include "timer.h"

#define MAX_EVENTS 1024

struct eventloop_s
{
    platform_backend_t *backend;
    timer_wheel_t *timer_wheel;
    event_t *events;
    int running;
    int error;
};

eventloop_t *eventloop_create(void)
{
    eventloop_t *loop = calloc(1, sizeof(eventloop_t));
    if (!loop)
    {
        return NULL;
    }

    loop->backend = platform_init();
    if (!loop->backend)
    {
        free(loop);
        return NULL;
    }

    loop->timer_wheel = timer_wheel_create();
    if (!loop->timer_wheel)
    {
        platform_cleanup(loop->backend);
        free(loop);
        return NULL;
    }

    loop->events = calloc(MAX_EVENTS, sizeof(event_t));
    if (!loop->events)
    {
        timer_wheel_destroy(loop->timer_wheel);
        platform_cleanup(loop->backend);
        free(loop);
        return NULL;
    }

    return loop;
}

void eventloop_destroy(eventloop_t *loop)
{
    if (!loop)
        return;

    free(loop->events);
    timer_wheel_destroy(loop->timer_wheel);
    platform_cleanup(loop->backend);
    free(loop);
}

void event_init(event_t *ev, int fd, uint32_t events, event_callback_fn cb, void *arg)
{
    memset(ev, 0, sizeof(event_t));
    ev->fd = fd;
    ev->events = events;
    ev->callback = cb;
    ev->arg = arg;
    ev->timer_flags = TIMER_ONESHOT; // Default to one-shot timer
}

int eventloop_add_event(eventloop_t *loop, event_t *ev)
{
    if (!loop || !ev)
        return EV_ERROR;

    int ret = platform_add_fd(loop->backend, ev->fd, ev->events);
    if (ret != EV_OK)
    {
        return ret;
    }

    ev->active = 1;
    return EV_OK;
}

int eventloop_remove_event(eventloop_t *loop, event_t *ev)
{
    if (!loop || !ev)
        return EV_ERROR;

    int ret = platform_remove_fd(loop->backend, ev->fd);
    if (ret != EV_OK)
    {
        return ret;
    }

    if (ev->timeout > 0)
    {
        timer_wheel_remove(loop->timer_wheel, ev);
    }

    ev->active = 0;
    return EV_OK;
}

int eventloop_modify_event(eventloop_t *loop, event_t *ev, uint32_t new_events)
{
    if (!loop || !ev)
        return EV_ERROR;

    int ret = platform_modify_fd(loop->backend, ev->fd, new_events);
    if (ret != EV_OK)
    {
        return ret;
    }

    ev->events = new_events;
    return EV_OK;
}

int eventloop_add_timer(eventloop_t *loop, event_t *ev, uint64_t timeout_ms)
{
    if (!loop || !ev)
        return EV_ERROR;

    ev->timeout = timeout_ms;
    ev->timer_flags = TIMER_ONESHOT; // Ensure it's one-shot
    return timer_wheel_add(loop->timer_wheel, ev, timeout_ms);
}

int eventloop_add_periodic_timer(eventloop_t *loop, event_t *ev, uint64_t interval_ms)
{
    if (!loop || !ev)
        return EV_ERROR;

    ev->timeout = interval_ms;
    ev->timer_flags = TIMER_PERIODIC; // Set as periodic timer
    return timer_wheel_add(loop->timer_wheel, ev, interval_ms);
}

int eventloop_run(eventloop_t *loop)
{
    if (!loop)
        return EV_ERROR;

    loop->running = 1;
    while (loop->running)
    {
        // Process timers
        timer_wheel_process(loop->timer_wheel, loop);

        // Calculate wait timeout
        int timeout = timer_wheel_next_timeout(loop->timer_wheel);

        // Wait for events
        int nev = platform_wait(loop->backend, &loop->events, MAX_EVENTS, timeout);

        if (nev < 0)
        {
            if (errno == EINTR)
                continue;
            loop->error = errno;
            return EV_ERROR;
        }

        // Process events
        for (int i = 0; i < nev; i++)
        {
            event_t *ev = &loop->events[i];
            if (ev->callback)
            {
                ev->callback(loop, ev, ev->arg);
            }
        }
    }

    return EV_OK;
}

void eventloop_stop(eventloop_t *loop)
{
    if (loop)
    {
        loop->running = 0;
    }
}

const char *eventloop_strerror(int error_code)
{
    switch (error_code)
    {
    case EV_OK:
        return "Success";
    case EV_ERROR:
        return "General error";
    case EV_TIMEOUT_ERR:
        return "Operation timed out";
    case EV_INVALID_FD:
        return "Invalid file descriptor";
    default:
        return strerror(error_code);
    }
}