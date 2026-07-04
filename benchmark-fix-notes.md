# Benchmark Output Fix Notes

This note explains why the benchmark originally produced no visible output, what was changed, and why the current version works.

## What the old code looked like

The benchmark used a very large memory pool size:

```cpp
const size_t NUM_ALLOCATIONS = 1000000;
MemoryPool<Order, NUM_ALLOCATIONS> order_pool;
```

Inside `MemoryPool`, the storage was originally embedded directly in the object:

```cpp
Slot pool_[PoolSize];
```

That meant the `MemoryPool` object itself contained a huge array of slots. Even when the pool wrapper was moved to the heap, the array was still part of the object layout.

## What was wrong

The main problem was a stack overflow.

In the original design, `MemoryPool<Order, 1000000>` created a very large object because it stored one million slots inline. On Windows, that was too large for the stack when the object lived in a local variable, so the program crashed before it could print the benchmark results.

That is why the executable seemed to show no output. The process was dying before the benchmark could finish, and in some cases even before the first `std::cout` output was visible.

There was also a follow-up bug after moving the pool to heap-owned storage. The deallocation logic still treated the storage as if it were a raw array:

```cpp
size_t index = reinterpret_cast<Slot*>(ptr) - pool_;
```

Once `pool_` became a smart pointer, that line no longer made sense.

## What changed

### 1. The pool storage moved off the stack

The internal slot buffer now uses heap-owned storage:

```cpp
std::unique_ptr<Slot[]> pool_;
```

The constructor now allocates the slots dynamically:

```cpp
pool_ = std::make_unique<Slot[]>(PoolSize);
```

This keeps the `MemoryPool` object itself small, while still giving it a contiguous block of storage for all slots.

### 2. Pointer math in `deallocate` was updated

Because `pool_` is now a `std::unique_ptr<Slot[]>`, the raw buffer is accessed with `get()`:

```cpp
size_t index = reinterpret_cast<Slot*>(ptr) - pool_.get();
```

That fixes the compile-time error and makes the slot index calculation work again.

## Why the current code works

The current design works for two reasons:

1. The big slot buffer is no longer embedded directly in the `MemoryPool` object, so the program does not blow the stack just by creating the pool.
2. Allocation and deallocation still operate on a single contiguous block of slots, so the pool keeps the same fast behavior and remains efficient.

That is why the benchmark now runs and prints output like:

```text
Starting benchmark for 1000000 allocations...

Standard C++ (new/delete): ... ms
Custom Memory Pool:        ... ms

Performance Gain: ...x faster!
```

## Simple summary

Old version:

- Stored one million slots directly inside the object
- Caused a stack overflow
- Program crashed before showing results

Current version:

- Stores the slots on the heap
- Keeps the pool object small
- Fixes the deallocation pointer calculation
- Runs successfully and prints benchmark output

## Files involved

- [main.cpp](main.cpp)
- [MemoryPool.h](MemoryPool.h)
