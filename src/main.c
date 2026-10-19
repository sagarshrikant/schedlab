#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "cmdline.h"
#include "common.h"

/* Exit codes: 0 ok, 1 test could not run, 2 bad command line. */
#define EXIT_RUN_FAILED 1
#define EXIT_BAD_ARGS 2

static const char *const scenario_names[] = {"none", "s1",    "s2", "s3",
                                             "s4",   "s4net", "s5", "sender"};

static int check_range(const char *option, int64_t value, int64_t lo, int64_t hi)
{
    if (value < lo || value > hi) {
        fprintf(stderr, "schedlab: --%s: %lld is out of range (%lld..%lld)\n", option,
                (long long)value, (long long)lo, (long long)hi);
        return -1;
    }
    return 0;
}

/*
 * The checks cliforge 0.4.0 does not do yet (see the note in cli/schedlab.cf):
 * missing --scenario, --nice (negative range), ranges of quantities, cpu number.
 * Positive integer ranges (prio options, port) are checked by the generated parser.
 * Remove a check here once cliforge enforces it. Returns 0 if all is fine.
 */
static int args_validate(const struct sl_cmdline *a)
{
    int bad = 0;

    if (a->scenario == SL_SCENARIO_ID_NONE) {
        fprintf(stderr, "schedlab: --scenario is required (s1 s2 s3 s4 s4net s5 sender)\n");
        bad = -1;
    }
    bad |= check_range("cpu", a->cpu, 0, platform_cpu_count() - 1);
    bad |= check_range("nice", a->nice, -20, 19);
    /* Quantities are checked in their base unit: ns and Hz. */
    bad |= check_range("duration (ns)", (int64_t)sl_duration_to_ns(&a->duration), 1000000000LL,
                       3600LL * 1000000000LL);
    bad |=
        check_range("period (ns)", (int64_t)sl_duration_to_ns(&a->period), 100000LL, 1000000000LL);
    bad |= check_range("rate (Hz)", (int64_t)sl_frequency_to_hz(&a->rate), 1, 100000);
    return bad;
}

/*
 * The control thread is main(). It runs on its own core at FIFO 99, so it
 * can always stop the test, and its memory is locked so it never page-faults.
 * Returns 0 or -1 (message printed).
 */
static int setup_control_thread(int control_cpu)
{
    struct sched_param sp = {.sched_priority = SL_CONTROL_PRIO};
    int err;

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        fprintf(stderr, "schedlab: mlockall: %s\n", strerror(errno));
        return -1;
    }
    err = platform_pin_thread(pthread_self(), control_cpu);
    if (err != 0) {
        fprintf(stderr, "schedlab: pin control thread to cpu %d: %s\n", control_cpu, strerror(err));
        return -1;
    }
    err = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
    if (err != 0) {
        fprintf(stderr, "schedlab: set FIFO %d for the control thread: %s\n", SL_CONTROL_PRIO,
                strerror(err));
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    struct sl_cmdline args;
    int rc = sl_cmdline_parse(argc, argv, &args);
    int ncpu;
    int control_cpu;

    if (rc > 0) {
        return 0; /* --help or --version was printed */
    }
    if (rc < 0) {
        fprintf(stderr, "Try 'schedlab --help'.\n");
        return EXIT_BAD_ARGS;
    }
    if (args_validate(&args) != 0) {
        return EXIT_BAD_ARGS;
    }

    ncpu = platform_cpu_count();
    if (ncpu < 2) {
        fprintf(stderr, "schedlab: needs at least 2 cores (test core + control core)\n");
        return EXIT_RUN_FAILED;
    }
    control_cpu = (int)((args.cpu + 1U) % (unsigned)ncpu);

    if (!args.quiet) {
        platform_print_info();
        printf("scenario   : %s\n", scenario_names[args.scenario]);
        printf("cores      : test cpu %u, control cpu %d (of %d)\n", args.cpu, control_cpu, ncpu);
    }
    if (setup_control_thread(control_cpu) != 0) {
        fprintf(stderr, "schedlab: %s\n", platform_privilege_hint());
        return EXIT_RUN_FAILED;
    }

    fprintf(stderr, "schedlab: scenario %s is not implemented yet\n",
            scenario_names[args.scenario]);
    return EXIT_RUN_FAILED;
}
