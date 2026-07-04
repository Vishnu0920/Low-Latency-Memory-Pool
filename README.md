# LowLatency_MemoryPool

High-performance, fixed-size C++ memory pool example and a tiny benchmark demonstrating why a pre-allocated pool can be faster and more deterministic than repeated OS allocations.

---

## TL;DR

- This project implements a fixed-size memory pool that pre-allocates a contiguous block of slots and reuses them with O(1) allocate/deallocate operations.
- A large inline array caused a stack overflow on Windows; the pool now allocates its slot buffer on the heap (via `std::unique_ptr<Slot[]>`).

---

## Files

- `MemoryPool.h` — templated memory pool implementation (uses placement `new`, intrusive free-list).
- `main.cpp` — small benchmark that compares `new/delete` vs the `MemoryPool` for `NUM_ALLOCATIONS` objects.
- `benchmark-fix-notes.md` — notes describing the original issue and the applied fix.

---

## Build & Run (example)

On a machine with `g++` (MinGW/MSYS2) installed:

```bash
g++ -std=c++17 -O2 main.cpp -o benchmark.exe
./benchmark.exe
```

Expected output (times will vary):

```
Starting benchmark for 1000000 allocations...

Standard C++ (new/delete): 34.50 ms
Custom Memory Pool:        4.15 ms

Performance Gain: 8.31x faster!
```

---

## What went wrong (brief)

The original `MemoryPool` used an inline array (`Slot pool_[PoolSize];`). With `PoolSize = 1'000'000` the `MemoryPool` object became very large. Creating that object (on the stack as a local variable) caused a stack probe / overflow and the program crashed before it could show benchmark output.

After moving the slot buffer to the heap (`std::unique_ptr<Slot[]> pool_`) the `MemoryPool` object is small and heap allocation no longer triggers a stack overflow.

Also fixed: pointer arithmetic in `deallocate()` now subtracts from `pool_.get()` (the raw buffer), not from the `unique_ptr` itself.

---

## Implementation notes

- The pool uses a union to hold either an object or the `next_free_index` when the slot is free — this avoids extra per-slot metadata.
- `allocate(...)` uses placement new to construct an object in an available slot.
- `deallocate(ptr)` calls the destructor explicitly and pushes the slot back to the free list.

---

## Where to look

- See `MemoryPool.h` for the pool implementation and the exact fixes.
- See `benchmark-fix-notes.md` for a readable before/after explanation.

---

If you want, I can also:

- Add a CMakeLists.txt for cross-platform builds.
- Add a small test harness that runs the benchmark with different `NUM_ALLOCATIONS` values.
- Convert `benchmark-fix-notes.md` into a short changelog entry.

Tell me which you'd like next.