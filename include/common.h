#ifndef SCHEDLAB_COMMON_H
#define SCHEDLAB_COMMON_H

#include <pthread.h>

/* The main thread runs at this FIFO priority on a core of its own,
 * so it can always stop a test. */
#define SL_CONTROL_PRIO 99

/*
 * Platform layer. One implementation per OS: src/platform_linux.c now,
 * src/platform_qnx.c later. Everything else in schedlab is plain POSIX.
 */

/* Number of online CPU cores (at least 1). */
int platform_cpu_count(void);

/* Pin a thread to one core. Returns 0 or an errno value. */
int platform_pin_thread(pthread_t thread, int cpu);

/* Print OS, kernel type and RT throttling info (or the QNX equivalent). */
void platform_print_info(void);

/* One line telling the user what privilege real-time scheduling needs. */
const char *platform_privilege_hint(void);

#endif /* SCHEDLAB_COMMON_H */
