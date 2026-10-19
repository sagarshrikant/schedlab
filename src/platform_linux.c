/* Linux platform layer. _GNU_SOURCE is needed for pthread_setaffinity_np(). */
#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "common.h"

#define RT_RUNTIME_FILE "/proc/sys/kernel/sched_rt_runtime_us"
#define RT_PERIOD_FILE "/proc/sys/kernel/sched_rt_period_us"
#define RT_SYSFS_FILE "/sys/kernel/realtime"

int platform_cpu_count(void)
{
    long n = sysconf(_SC_NPROCESSORS_ONLN);

    return (n > 0) ? (int)n : 1;
}

int platform_pin_thread(pthread_t thread, int cpu)
{
    cpu_set_t set;

    if (cpu < 0 || cpu >= CPU_SETSIZE) {
        return EINVAL;
    }
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    return pthread_setaffinity_np(thread, sizeof(set), &set);
}

/* Read the first line of a file without the newline. Returns 0 on success. */
static int read_first_line(const char *path, char *buf, size_t size)
{
    FILE *f = fopen(path, "r");

    if (f == NULL) {
        return -1;
    }
    if (fgets(buf, (int)size, f) == NULL) {
        (void)fclose(f);
        return -1;
    }
    (void)fclose(f);
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

/* PREEMPT_RT: /sys/kernel/realtime holds "1", else look in `uname -v`. */
static int kernel_is_preempt_rt(const struct utsname *u)
{
    char line[16];

    if (read_first_line(RT_SYSFS_FILE, line, sizeof(line)) == 0) {
        return line[0] == '1';
    }
    return strstr(u->version, "PREEMPT_RT") != NULL;
}

void platform_print_info(void)
{
    struct utsname u;
    char runtime[32] = "unavailable";
    char period[32] = "unavailable";

    if (uname(&u) != 0) {
        (void)strcpy(u.sysname, "Linux");
        u.release[0] = '\0';
        u.version[0] = '\0';
    }
    (void)read_first_line(RT_RUNTIME_FILE, runtime, sizeof(runtime));
    (void)read_first_line(RT_PERIOD_FILE, period, sizeof(period));

    printf("platform   : %s %s\n", u.sysname, u.release);
    printf("kernel     : %s\n", u.version);
    printf("PREEMPT_RT : %s\n", kernel_is_preempt_rt(&u) ? "yes" : "no");
    printf("RT throttle: sched_rt_runtime_us=%s sched_rt_period_us=%s (-1 = off)\n", runtime,
           period);
}

const char *platform_privilege_hint(void)
{
    return "real-time scheduling on Linux needs root or CAP_SYS_NICE (and CAP_IPC_LOCK or a "
           "high RLIMIT_MEMLOCK for mlockall)";
}
