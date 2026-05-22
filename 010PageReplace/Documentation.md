# Page Replacement Simulator

A step-by-step simulation of virtual memory page replacement algorithms. Given a page reference sequence and a fixed number of physical frames, the program simulates FIFO, MIN (Optimal), LRU, and Second Chance policies, displaying the RAM state and a disk access animation for each step. The user advances through each reference by pressing Enter.

## Project Structure

```text
010PageReplacement/
├── app/
│   └── main.c          # Entry point: CLI argument parsing, logger dispatch
├── lib/
│   ├── logger.c        # Step-by-step display, disk animation, algorithm loop
│   └── logger.h        # Logger public interface
├── os/
│   ├── RAM.c           # Physical frame array: init, free, view copy
│   ├── RAM.h           # Frame type, FRAME_EMPTY constant, extern declarations
│   ├── scheduler.c     # FIFO, MIN, LRU, Second Chance — one step per call
│   └── scheduler.h     # access_result type, algorithm declarations, hit/miss counters
├── out/                # Compiled object files and binary
└── Makefile
```

## Layered Architecture

| Layer | Components | Responsibility |
|---|---|---|
| **Application** | `app/main.c` | Parses CLI args, reads reference sequence, calls `logger_init` and `logger_run` |
| **Library** | `lib/logger.c` | Drives the step loop, renders RAM map, animates disk transfers |
| **OS — Scheduler** | `os/scheduler.c` | Implements replacement policies; one access attempt per call, returns hit/miss + evicted page |
| **OS — RAM** | `os/RAM.c` | Owns the physical `frames[]` array; scheduler reads/writes it directly |

## Replacement Algorithms

All algorithms operate on the same reference sequence and frame count.

**FIFO** loads pages in arrival order. On eviction, removes the page that has been in memory the longest. Simple but susceptible to Belady's anomaly.

**MIN (Optimal)** evicts the page whose next use appears farthest in the future. Requires full lookahead into the reference string — not implementable in real OSes, but serves as a theoretical lower bound for misses. Tie-break: smallest page id.

**LRU** evicts the page that was least recently accessed. Implemented with a doubly linked list; on each access the referenced page moves to the front. Tail is always the eviction candidate. Tie-break: smallest page id.

**Second Chance** is a FIFO variant with a reference bit. When a page is about to be evicted, if its reference bit is set it gets one more chance (bit cleared, moved to back of queue) instead of being replaced. Approximates LRU with lower overhead.

## Display

### RAM Map

Frames are displayed in rows of up to 10, with borders sized to the actual row width:

```
/---------------------------\
||000:007||001:000||002:001||
\---------------------------/
```

Each cell shows `|frame_index:page_number|`. The accessed or changed frame is highlighted in green (hit / new load) or red (eviction).

### Disk Animation

On a page fault with eviction, a disk transfer animation plays:

```
  Before:
  /---------------------------\
  ||000:007||001:000||002:001||    (evicted frame in red)
  \---------------------------/
     --->
          .---.
         /     \
        | (( )) |
         \     /
          '---'
     <---
  After:
  /---------------------------\
  ||000:007||001:000||002:004||    (loaded frame in green)
  \---------------------------/
```

The arrow animates one character at a time at 0.2s intervals, forward then backward. The terminal is cleared on each step so only the current state is visible.

## Building and Running

```bash
# Build
make TARGET=linux PROJECT=010PageReplace

# Run with inline sequence
./010PageReplace/bin/main -N 3 -query "7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1" -a all

# Run from file (one page number per line, hex or decimal)
./010PageReplace/bin/main -N 3 -f path_to/testcase.txt -a lru

# Run only one algorithm
./010PageReplace/bin/main -N 4 -query "1 2 3 4 1 2 5" -a fifo

# Clean
make clean
```

### Arguments

| Flag | Description | Default |
|---|---|---|
| `-N` | Number of physical frames | `3` |
| `-query` | Space-separated page reference sequence | — |
| `-f` | Path to file with one page number per line | — |
| `-a` | Algorithm: `fifo` \| `min` \| `lru` \| `second_chance` \| `all` | `all` |
| `-help` | Show help message | — |

The `-query` flag takes priority over `-f` if both are given. The reference sequence is required — no default value.

## Output Summary

After each algorithm completes, the simulator prints:

- Total hits and misses
- Hit rate as a percentage

Expected miss ranking for the lab reference dataset (N=3, 20 references): MIN ≤ LRU ≤ FIFO — MIN is optimal by definition.
