# CPU Scheduling Algorithms Simulation

A simulation of four CPU scheduling algorithms — FIFO, Round Robin, SJF, and SRTF — implemented in C. The simulation generates random threads with random burst and arrival times, runs all four algorithms on the same dataset, and produces a colored ASCII Gantt chart, a timestamped execution log, and a statistics file for each algorithm.

## Project Structure

```text
008Scheduling/
├── source/
│   ├── main.c          # Entry point: thread generation, algorithm dispatch
│   ├── output.c        # Terminal output, Gantt chart, log/stats file writing
│   └── output.h
├── lib/
│   ├── random.c        # LCG random number generator
│   └── random.h
├── os/
│   ├── scheduler.c     # FIFO, Round Robin, SJF, SRTF implementations
│   └── scheduler.h     # thread_t, stats_t, gantt_t types and algorithm declarations
├── bin/                # Compiled object files and binary
└── logs/
    ├── run.log         # Timestamped execution history (all algorithms)
    └── stats.log       # Statistics per algorithm 
```

## Layered Architecture

| Layer | Components | Responsibility |
|---|---|---|
| **Application** | `source/` | Thread generation, program flow, output formatting |
| **Library** | `lib/` | Reusable utilities (random number generator) |
| **OS** | `os/` | Scheduling logic — algorithm implementations and data structures |

## Scheduling Algorithms

All four algorithms operate on the same randomly generated thread dataset.

**FIFO** sorts threads by arrival time and runs each to completion before starting the next. Simple but can cause long waits for short jobs arriving behind long ones.

**Round Robin** gives each thread a fixed time quantum (`RR_QUANTUM = 2s`), cycling through the ready queue. Fairer than FIFO under equal burst times, but higher average turnaround due to context switch overhead.

**SJF (Shortest Job First)** is non-preemptive — at each scheduling point it selects the arrived thread with the shortest burst time. Minimizes average waiting time for batch workloads but can starve long jobs.

**SRTF (Shortest Remaining Time First)** is the preemptive variant of SJF. At every time unit it selects the thread with the least remaining time, preempting the current one if a shorter job arrives. Optimal for average waiting time but incurs frequent context switches.

## Output

### Terminal

At startup, a thread table is printed showing each thread's color, arrival time, and burst time. After each algorithm runs, the following are printed to the terminal:

- Timestamped execution log (arrived / started / completed events)
- ASCII Gantt chart, each thread in a distinct ANSI color
- Statistics (waiting times, turnaround times, averages)

**Gantt chart format:**
```
/------------------------------------------------------------\
 ...|||||||||||||||||||||||||||||||||||||||||||||||||||||||||
\------------------------------------------------------------/ FIFO
```
Each `|` is colored by thread ID. Dots represent idle CPU time.

### Log files (`logs/`)

`run.log` — full timestamped execution history for all algorithms, appended sequentially.

`stats.log` — waiting times, turnaround times, and averages per algorithm.

## Building and Running

```bash
# Build and run
make run PROJECT=008Scheduling TARGET=linux

# Build only
make PROJECT=008Scheduling TARGET=linux

# Clean
make clean PROJECT=008Scheduling
```
For more details, consult the README of the repository.

## Statistics Summary

For each algorithm the simulation reports:

- **Waiting time** per thread: time from arrival to first start (FIFO/SJF) or total accumulated waiting time (RR/SRTF)
- **Turnaround time** per thread: finish time minus arrival time
- **Average waiting time** and **average turnaround time** across all threads

Expected ranking by average waiting time (best to worst): SRTF < SJF < RR < FIFO
