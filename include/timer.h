#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "eventloop.h"

// Timer wheel structure
typedef struct timer_wheel_s timer_wheel_t;

// Initialize timer wheel
timer_wheel_t* timer_wheel_create(void);

// Destroy timer wheel
void timer_wheel_destroy(timer_wheel_t* wheel);

// Add timer event
int timer_wheel_add(timer_wheel_t* wheel, event_t* ev, uint64_t timeout_ms);

// Remove timer event
int timer_wheel_remove(timer_wheel_t* wheel, event_t* ev);

// Get next timeout
int timer_wheel_next_timeout(timer_wheel_t* wheel);

// Process expired timers
void timer_wheel_process(timer_wheel_t* wheel, eventloop_t* loop);

#endif // TIMER_H
