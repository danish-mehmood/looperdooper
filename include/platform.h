#ifndef PLATFORM_H
#define PLATFORM_H

#include "eventloop.h"

// Platform-specific event backend structure
typedef struct platform_backend_s platform_backend_t;

// Platform initialization
platform_backend_t* platform_init(void);

// Platform cleanup
void platform_cleanup(platform_backend_t* backend);

// Add file descriptor to monitoring
int platform_add_fd(platform_backend_t* backend, int fd, uint32_t events);

// Remove file descriptor from monitoring
int platform_remove_fd(platform_backend_t* backend, int fd);

// Modify monitored events for file descriptor
int platform_modify_fd(platform_backend_t* backend, int fd, uint32_t events);

// Wait for events with timeout
int platform_wait(platform_backend_t* backend, event_t** events, int max_events, int timeout_ms);

#endif // PLATFORM_H
