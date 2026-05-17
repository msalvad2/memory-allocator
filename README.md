# Memory Allocator

A `malloc`/`free`/`realloc` implementation in C built from scratch using `sbrk`
and `mmap`. Covers the core design decisions behind real-world allocators —
boundary tag coalescing, block splitting, and a dual allocation path that matches
the strategy used by glibc.

Verified with 55 checks and a 10,000-operation stress test. Zero leaks, zero
Valgrind errors.

## Block Layout

Every allocation is wrapped in a header and footer:

    ┌──────────────┬───────────────┬──────────────┐
    │    header    │   user data   │    footer    │
    │  (size|free) │               │    (size)    │
    └──────────────┴───────────────┴──────────────┘
    ^                              ^
    block                        block + HEADER_SIZE + size

The footer mirrors the header's size, enabling O(1) access to the previous
block without traversing from heap start — the boundary tag technique.

## How It Works

`malloc` searches the heap for a free block first. If one is found and larger
than needed, it is split — the remainder becomes a new free block. If no block
qualifies, `sbrk` extends the heap. Requests at or above 128KB bypass the heap
entirely and use `mmap`, matching glibc's threshold. `free` coalesces adjacent
free blocks immediately in both directions to prevent external fragmentation.
`realloc` handles five cases — including in-place grow by absorbing an adjacent
free block — to avoid unnecessary copies.

## Features

- Explicit free list with first-fit search and block reuse
- Boundary tag coalescing — forward, backward, and three-way
- Block splitting with minimum size enforcement to prevent invalid blocks
- `mmap` path for allocations >= 128KB with immediate OS reclamation via `munmap`
- Full `realloc` — five cases including in-place grow/shrink
- Overflow guard against `SIZE_MAX` and alignment wrap-around inputs

## Build

Requires GCC and Valgrind headers:

    sudo apt install valgrind
    make
    ./test

## Test Coverage

55 checks across:
- Edge cases: `malloc(0)`, `malloc(SIZE_MAX)`, `free(NULL)`, `realloc(NULL, 0)`
- Pointer alignment across 6 sizes including one mmap allocation
- Block reuse, forward/backward/three-way coalescing, block splitting
- All five `realloc` cases with byte-level data preservation checks
- 10,000-operation stress test with sentinel-based corruption detection