#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <stdint.h>
#include <sys/types.h>

// Event types - Using bit flags (one bit per event type) to allow combining multiple events
// Binary:  Hex:    Usage:
// 0001  = 0x01  - Read events (socket ready for reading)
// 0010  = 0x02  - Write events (socket ready for writing)
// 0100  = 0x04  - Timer events (timeout/interval triggered)
// 1000  = 0x08  - Signal events (OS signal received)
// Events can be combined using bitwise OR, e.g.: EV_READ | EV_WRITE = 0x03
#define EV_READ 0x01
#define EV_WRITE 0x02
#define EV_TIMER 0x04
#define EV_SIGNAL 0x08

// Timer flags
#define TIMER_ONESHOT 0x00
#define TIMER_PERIODIC 0x01

// Error codes
#define EV_OK 0
#define EV_ERROR -1
#define EV_TIMEOUT_ERR -2
#define EV_INVALID_FD -3

// Forward declarations
typedef struct eventloop_s eventloop_t;
typedef struct event_s event_t;

// Callback function type
typedef void (*event_callback_fn)(eventloop_t *loop, event_t *ev, void *arg);

// Event structure
struct event_s
{
    int fd;                     // File descriptor
    uint32_t events;            // Event types to monitor
    event_callback_fn callback; // Callback function
    void *arg;                  // User data
    uint64_t timeout;           // Timeout in milliseconds (0 for no timeout)
    uint32_t timer_flags;       // Timer flags (TIMER_ONESHOT or TIMER_PERIODIC)
    struct event_s *next;       // For internal use
    int active;                 // Is event active?
};

// Public API

// Create a new event loop
eventloop_t *eventloop_create(void);

// Destroy event loop
void eventloop_destroy(eventloop_t *loop);

// Initialize an event structure
void event_init(event_t *ev, int fd, uint32_t events, event_callback_fn cb, void *arg);

// Add an event to the loop
int eventloop_add_event(eventloop_t *loop, event_t *ev);

// Remove an event from the loop
int eventloop_remove_event(eventloop_t *loop, event_t *ev);

// Modify existing event
int eventloop_modify_event(eventloop_t *loop, event_t *ev, uint32_t new_events);

// Add a timer event
int eventloop_add_timer(eventloop_t *loop, event_t *ev, uint64_t timeout_ms);

// Add a periodic timer event
int eventloop_add_periodic_timer(eventloop_t *loop, event_t *ev, uint64_t interval_ms);

// Run the event loop
int eventloop_run(eventloop_t *loop);

// Stop the event loop
void eventloop_stop(eventloop_t *loop);

// Get last error message
const char *eventloop_strerror(int error_code);

#endif // EVENTLOOP_H