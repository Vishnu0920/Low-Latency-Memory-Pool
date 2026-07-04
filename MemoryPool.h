#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <memory>
#include <utility>
#include <stdexcept>
#include <new>

// A templated, fixed-size memory pool.
// T is the object type (e.g., an Order)
// PoolSize is the maximum number of objects the pool can hold.
template <typename T, size_t PoolSize>
class MemoryPool {
private:
    // The Intrusive Free List trick.
    // A slot holds EITHER your object, OR the index of the next free slot.
    // This requires no extra memory overhead for tracking free blocks!
    union Slot {
        T object;
        size_t next_free_index;
        
        // We have to explicitly define constructors/destructors for unions 
        // that contain non-trivial types (like our object T might be).
        Slot() {} 
        ~Slot() {}
    };

    // Our pre-allocated contiguous memory block
    std::unique_ptr<Slot[]> pool_;
    
    // The index of the first available empty slot
    size_t free_head_;

public:
    // -------------------------------------------------------------
    // TODO 1: Implement the Constructor
    // -------------------------------------------------------------
    // Goal: Initialize the free list. 
    // Loop through pool_ and make every slot's 'next_free_index' 
    // point to the next slot (e.g. pool_[0].next_free_index = 1).
    // Set free_head_ to 0.
    MemoryPool() {
        pool_ = std::make_unique<Slot[]>(PoolSize);

        for (size_t i = 0; i < PoolSize - 1; ++i) {
            pool_[i].next_free_index = i + 1;
        }

        pool_[PoolSize - 1].next_free_index = static_cast<size_t>(-1); // Mark the end of the free list
        free_head_ = 0; // Start with the first slot as free

    }

    // -------------------------------------------------------------
    // TODO 2: Implement Allocate
    // -------------------------------------------------------------
    // Goal: Get a free slot and construct the object inside it.
    // 1. Check if free_head_ is valid (pool is not full).
    // 2. Save the current free_head_ index.
    // 3. Update free_head_ to the next_free_index of the current slot.
    // 4. Use Placement New to construct the object in the saved slot.
    // 5. Return a pointer to the newly constructed object.
    template <typename... Args>
    T* allocate(Args&&... args) {
        // Your code here
        // Placement new syntax hint: new (&pool_[index].object) T(std::forward<Args>(args)...);
        if (free_head_ == static_cast<size_t>(-1)) {
            throw std::bad_alloc();
        }

        size_t index = free_head_;
        free_head_ = pool_[index].next_free_index;
        new (&pool_[index].object) T(std::forward<Args>(args)...);
        return &pool_[index].object;
    }

    // -------------------------------------------------------------
    // TODO 3: Implement Deallocate
    // -------------------------------------------------------------
    // Goal: Destroy the object and return the slot to the free list.
    // 1. Explicitly call the destructor: ptr->~T();
    // 2. Calculate the index of this pointer relative to pool_ array.
    // 3. Set this slot's next_free_index to the current free_head_.
    // 4. Set free_head_ to the index of this slot.
    void deallocate(T* ptr) {
        // Your code here
        // Index calculation hint: size_t index = reinterpret_cast<Slot*>(ptr) - pool_;
        if (ptr == nullptr) {
            return; // Do nothing for null pointer
        }

        ptr->~T(); // Call the destructor explicitly
        
        //convert ptr to Slot* and calculate the index
        //ptr is a pointer to T, but we need to treat it as a pointer to Slot to calculate the index in the pool
        //reinterpret_cast<Slot*>(ptr) converts the T* pointer to a Slot* pointer, allowing us to calculate the index in the pool_ array.
        size_t index = reinterpret_cast<Slot*>(ptr) - pool_.get();

        // Safety check: Ensure the pointer actually belongs to our pool
        if (index >= PoolSize) {
            throw std::invalid_argument("Pointer does not belong to this memory pool.");
        }

        pool_[index].next_free_index = free_head_;
        free_head_ = index;
    }
};

#endif // MEMORY_POOL_H