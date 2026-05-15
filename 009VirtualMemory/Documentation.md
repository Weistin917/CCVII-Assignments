# Virtual Memory Simulator

A simulation of virtual-to-physical address translation using paged memory management. The program initializes a physical RAM with randomly pre-occupied frames, loads a process into the remaining free frames, and translates a batch of virtual addresses from a file through a page table. The RAM state is visualized in the terminal with a color-coded memory map, and each address translation is displayed step by step with user-paced progression.

## Project Structure

```text
009VirtualMemory/
├── source/
│   └── main.c              # Entry point: argument parsing, batch translation loop
├── os/
│   ├── mmu.c               # Memory Management Unit: frame allocation, address translation
│   ├── mmu.h               # MMU public interface + frame struct definition
│   ├── RAM.h               # Physical RAM: frames[] array and frame state constants
│   ├── page_table.c        # Page table: creation, entry set/get
│   └── page_table.h        # Page table opaque type and interface
├── lib/
│   ├── memio.c             # Display layer: RAM map, address translation output
│   ├── memio.h             # memio public interface
│   ├── random.c            # LCG random number generator
│   └── random.h
└── example_addresses.txt   # Default input file with virtual addresses to translate
```

## Layered Architecture

| Layer | Components | Responsibility |
|---|---|---|
| **Application** | `app/main.c` | CLI argument parsing, seeds RNG, drives batch translation loop |
| **Display** | `lib/memio.c` | Formats and prints RAM map, summary, address translations. Holds a copy (`current_ram_view`) of RAM state — never accesses `frames[]` directly |
| **MMU** | `os/mmu.c` | Owns the page table and `frames[]`. Handles frame allocation, process loading, and virtual-to-physical translation |
| **Page Table** | `os/page_table.c` | Opaque page table structure. Maps virtual page numbers (VPN) to physical frame numbers (PFN) |
| **RAM** | `os/RAM.h` | Defines the physical `frames[]` array and frame state constants (`FREE`, `PREOCCUPIED`, `OCCUPIED`) |
| **Library** | `lib/random.c` | Seeded LCG RNG used for random pre-occupation of frames |

## How It Works

### Address Format

Both virtual and physical addresses are 16-bit, split into two 8-bit fields:

```
 15        8 7        0
 ┌──────────┬──────────┐
 │   VPN   │  Offset │   Virtual Address
 └──────────┴──────────┘
 ┌──────────┬──────────┐
 │   PFN   │  Offset │   Physical Address
 └──────────┴──────────┘
```

Translation: `PA = (page_table[VPN].PFN << 8) | offset`

### Execution Flow

1. **Argument parsing** — `-V` sets the number of virtual pages, `-seed` sets the RNG seed for reproducibility, `-f` specifies the address batch file.
2. **RAM initialization** — a random number of frames (between 10% and 60% of total) are marked `PREOCCUPIED`, simulating memory already in use by other processes.
3. **Process loading** — the process's virtual pages are mapped one-to-one to the first available free frames, marking them `OCCUPIED` and recording the VPN→PFN mapping in the page table.
4. **Batch translation** — each line of the input file is read as a virtual address (hex with `0x` prefix or decimal), translated to a physical address, and printed. The user presses Enter to advance to the next address.

### Input File Format

Each line contains one virtual address, accepted in either decimal or hex (`0x` prefix):

```
0xFF00
0x1A2B
255
4096
```

### RAM Map Display

The memory map prints all 100 frames in rows of 10, color-coded by state:

```
 /------...------\
|  000:    |  001:    | ... |  009:    |
|  010:0x01| ...
 \------...------/
```

| Color | State | Meaning |
|---|---|---|
| Green | `FREE` | Available frame |
| Red | `PREOCCUPIED` | In use by another process |
| Cyan | `OCCUPIED` | Mapped to a virtual page of the loaded process |

Each `OCCUPIED` frame shows its mapped virtual page number in hex (e.g. `0x01`).

## Building and Running

```bash
# Everything should be executed from the root directory of the repository.
# Build
make TARGET=linux PROJECT=009VirtualMemory

# Run with defaults (49 virtual pages, random seed, example_addresses.txt)
./009VirtualMemory/bin/main

# Run with specific virtual pages, seed, and address file
./009VirtualMemory/bin/main -V 20 -seed 42 -f my_addresses.txt

# Show help
./009VirtualMemory/bin/main -help
```

## Key Design Decisions

**`frame` struct lives in `mmu.h`, not `RAM.h`** — `memio.c` needs the `frame` type to hold its RAM view copy, but must not have access to the live `frames[]` array. Defining the struct in `mmu.h` (the public MMU interface) lets `memio.c` include only `mmu.h` and remain decoupled from the physical RAM.

**`memio.c` works on a copy** — `get_RAM_view()` copies `frames[]` into `current_ram_view` via `memcpy`. The display layer never reads or writes the real RAM directly.

**Page table is opaque** — `pt` is a forward-declared struct defined only in `page_table.c`. Callers interact with it exclusively through `pt_create`, `pt_destroy`, `pt_get_pfn`, and `pt_set_entry`, keeping internal layout hidden.

**Address input is format-agnostic** — `strtol` with base `0` auto-detects `0x`-prefixed hex and plain decimal, so the input file can mix both formats freely.
