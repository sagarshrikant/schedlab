# schedlab

Show how thread scheduling behaves on Linux, PREEMPT_RT and QNX

**Version:** 0.1.0

## Options

### Test setup

#### `--scenario`

Scenario to run: s1 s2 s3 s4 s4net s5 sender.

#### `--cpu`

Core that all test threads are pinned to.

#### `--duration`

Test length, for example 10s.

#### `--csv`

Write results to this CSV file.

#### `--quiet`

Print only the final result.

### Scheduling

#### `--policy`

Scheduling policy of the test threads.

#### `--prio`

Priority of the test threads (fifo, rr).

#### `--nice`

Nice value (policy other). Write --nice=-5 for negatives.

#### `--yield-every`

Hog calls sched_yield() every N iterations (0 = never).

### Priorities per role

#### `--hog-prio`

FIFO priority of the hog thread.

#### `--producer-prio`

FIFO priority of the producer thread (s4).

#### `--consumer-prio`

FIFO priority of the consumer thread (s4, s4net).

### Timing and network

#### `--period`

Period of the periodic thread, for example 1ms.

#### `--port`

UDP port (s4net, sender).

#### `--target`

Receiver IP address (sender).

#### `--rate`

Packets per second (sender), for example 1kHz.

