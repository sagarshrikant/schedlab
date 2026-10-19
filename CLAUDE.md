# CLAUDE.md — schedlab

## What this is
`schedlab` is a small POSIX C program that shows how thread scheduling really behaves on
**Linux (normal kernel)**, **Linux PREEMPT_RT** and **QNX 8**, using the same source code.
It backs a LinkedIn series and a public GitHub repo. Owner: Shrikant (embedded C, tech lead,
tracing background). Target: Raspberry Pi 4 (AArch64); must also build on x86-64 Linux.

Results must be visible **without any special tool**: printed stats and CSV only.

## How to work with me
- I am learning. Explain each design choice in 1–2 short lines. Simple English, short answers.
- Small steps. One milestone at a time; stop after each one and show me what changed.
- I review every change. Don't refactor beyond the current milestone.
- When unsure about scheduler behaviour, say so and propose a test, don't guess.

## Hard rules (no tracing in this repo)
- The repo is a plain example: clone, build, run, read stats. Nothing else.
- **No tracing of any kind:** no trace hooks or macros, no ftrace / `trace_marker`, no QNX
  tracelogger / `trace_logf`, no trace scripts, configs, instructions or trace files.
  Don't suggest adding them.
- Never mention any commercial tracer in code or comments. The owner writes all README text
  about tracing and tools himself.
- Keep the code easy to read at the key points: one clearly named function each for
  wakeup, produce, consume and hog loop, so the owner can study it locally.
- `local/` is git-ignored. Never read from it, list it, copy from it, or commit it.
- `scripts/check_clean.sh` must pass before every commit (also wired as a git pre-commit
  hook). It fails if `local/` content is staged or if any word from the git-ignored
  deny-list `local/denylist.txt` appears in tracked files.

## CLI
Use **cliforge** (github.com/sagarshrikant/cliforge) to generate the command-line parser.
First read the cliforge README and examples, then write the spec in `cli/`. Don't invent
cliforge syntax — ask me if something is unclear. Generated code goes in `build/` or
`src/generated/` as cliforge recommends.

Options (minimum):
`--scenario s1|s2|s3|s4|s4net|s5|sender`, `--cpu N` (test core), `--duration SEC`,
`--policy fifo|rr|other`, `--prio`, `--nice`, `--yield-every N`, `--hog-prio`,
`--producer-prio`, `--consumer-prio`, `--period-us`, `--port`, `--target IP`, `--rate HZ`,
`--csv FILE`, `--quiet`.

## Scenarios
All test threads are pinned to **one** core (`--cpu`). The main/control thread runs on a
different core at FIFO 99, so it can always stop the test. Memory locked with `mlockall`.
Threads created with `PTHREAD_EXPLICIT_SCHED`.

| ID | What it shows | Threads | Output |
| --- | --- | --- | --- |
| s1 | FIFO vs RR vs yield | 2 hogs, same prio; `--policy fifo|rr`, `--yield-every` | iterations per thread (CPU share), RR interval from `sched_rr_get_interval` |
| s2 | class beats nice | OTHER hog at nice −20 + FIFO 1 hog | iterations per thread; OTHER should get only the RT-throttling window |
| s3 | blocking vs hogging | FIFO 90 periodic (`clock_nanosleep`, `TIMER_ABSTIME`) + FIFO 70 hog | wakeup latency stats of the periodic thread, hog iterations |
| s4 | starvation via low-prio producer | FIFO 90 consumer waits on semaphore, FIFO 50 periodic producer, FIFO 70 hog starts after 1 s | consumer latency (post→wake), missed periods; rerun with producer 85 = fix |
| s4net | same with a real IRQ thread | FIFO 90 consumer blocks on UDP `recv`; packets from `sender` on another host; FIFO 70 hog | packet latency/gaps; on PREEMPT_RT change NIC IRQ thread prio with `scripts/irq_prio.sh` |
| s5 | wakeup latency | FIFO 99 periodic 1 ms; load from outside (`stress-ng`, ping flood) | min/avg/max, histogram (1 µs buckets, overflow bucket), CSV |
| sender | traffic for s4net | UDP sender at `--rate` | packet counter in payload |

Timestamps: `CLOCK_MONOTONIC`. Latency = actual wake − expected wake.
Hogs: busy loop, check stop flag + time every N iterations; count iterations.

## Platform notes
- Linux: needs root or `CAP_SYS_NICE`; print RT throttling values from
  `/proc/sys/kernel/sched_rt_runtime_us` and `sched_rt_period_us`; print whether kernel is
  PREEMPT_RT (`/sys/kernel/realtime` or `uname -v`).
- Pinning: Linux `pthread_setaffinity_np`; QNX `ThreadCtl(_NTO_TCTL_RUNMASK, ...)`.
- QNX: priorities above 63 need root/ability; print `pidin`-style hint at start.
- Keep platform code in `src/platform_linux.c` / `src/platform_qnx.c`, nothing else `#ifdef`'d.

## Layout
```
CMakeLists.txt
cmake/qnx-aarch64.cmake        # QNX toolchain file (qcc)
cli/                           # cliforge spec
include/  common.h
src/      main.c scenarios/*.c threads.c stats.c platform_*.c
scripts/  run_all.sh check_clean.sh irq_prio.sh install-hooks.sh
results/  (CSV, git-ignored except examples/)
docs/     README parts, pictures added by owner
local/    git-ignored
```

## Build and quality
- C11, POSIX. `-Wall -Wextra -Werror -O2`. Builds: Linux native (gcc, clang), Linux
  AArch64 cross, QNX 8 AArch64 (qcc).
- clang-format file in repo; run it before commit.
- No dynamic allocation after test start.
- `scripts/run_all.sh` runs s1–s5 with fixed settings and writes `results/<host>_<kernel>.csv`.

## Milestones (stop after each)
1. Skeleton: CMake, cliforge CLI, platform layer, builds on Linux. `check_clean.sh` +
   pre-commit hook in place.
2. s1, s2 working on Linux.
3. s3, s4 working on Linux.
4. s5 with histogram + CSV; `run_all.sh`.
5. s4net + sender + `irq_prio.sh`.
6. QNX build; run all on QNX.
7. README draft: purpose, how to build/run, what each scenario shows, expected results table.
   Leave result numbers, pictures and any tool/marketing section empty for the owner.

## Done when
All milestones done, `check_clean.sh` passes, and both normal and PREEMPT_RT Linux plus
QNX produce a CSV.
