#pragma once
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <new>

class Allocator
{
  public:
    // Allocate memory using std::malloc and track the allocation size
    static void* allocate(std::size_t size)
    {
        if (size == 0)
        {
            return nullptr;
        }

        void* ptr = std::malloc(size);
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        // Update statistics
        total_allocated.fetch_add(size, std::memory_order_relaxed);
        current_allocated.fetch_add(size, std::memory_order_relaxed);

        return ptr;
    }

    // Deallocate memory using std::free and track the deallocation size
    static void deallocate(void* ptr, std::size_t size)
    {
        if (!ptr)
        {
            return;
        }

        std::free(ptr);

        // Update statistics
        current_allocated.fetch_sub(size, std::memory_order_relaxed);
    }

    // Reallocate memory using std::realloc and track the size changes
    static void* reallocate(void* ptr, std::size_t old_size, std::size_t new_size)
    {
        if (new_size == 0)
        {
            deallocate(ptr, old_size);
            return nullptr;
        }

        void* new_ptr = std::realloc(ptr, new_size);
        if (!new_ptr)
        {
            throw std::bad_alloc();
        }

        if (new_size > old_size)
        {
            std::size_t size_diff = new_size - old_size;
            total_allocated.fetch_add(size_diff, std::memory_order_relaxed);
            current_allocated.fetch_add(size_diff, std::memory_order_relaxed);
        }
        else if (new_size < old_size)
        {
            std::size_t size_diff = old_size - new_size;
            current_allocated.fetch_sub(size_diff, std::memory_order_relaxed);
        }

        return new_ptr;
    }

    // Allocate memory and call constructor using new
    template <typename T, typename... Args> static T* create(Args&&... args)
    {
        void* memory = allocate(sizeof(T));
        return new (memory) T(std::forward<Args>(args)...);
    }

    // Allocate memory for T plus extra space, and call constructor for T
    template <typename T, typename... Args>
    static T* create_with_extra(std::size_t extra_size, Args&&... args)
    {
        // Allocate memory for T and extra space
        void* memory = allocate(sizeof(T) + extra_size);
        // Construct the object at the beginning of the allocated memory
        return new (memory) T(std::forward<Args>(args)...);
    }

    // Reallocate memory for T plus extra space, preserving the existing object
    template <typename TNEW, typename TOLD>
    static TNEW* recreate_with_extra(TOLD* ptr, std::size_t old_extra_size,
                                     std::size_t new_extra_size)
    {
        if (!ptr)
        {
            throw std::invalid_argument("Pointer must not be null");
        }

        std::size_t old_size = sizeof(TOLD) + old_extra_size;
        std::size_t new_size = sizeof(TNEW) + new_extra_size;
        void* new_memory = reallocate(ptr, old_size, new_size);

        // If reallocation changes the memory location, the existing object remains valid
        return static_cast<TNEW*>(new_memory);
    }

    // Destroy object and deallocate memory using delete
    template <typename T> static void destroy(T* ptr)
    {
        if (ptr)
        {
            ptr->~T();
            deallocate(ptr, sizeof(T));
        }
    }

    // Deallocate memory for T plus extra space
    template <typename T> static void destroy_with_extra(T* ptr, std::size_t extra_size)
    {
        if (ptr)
        {
            ptr->~T();
            deallocate(ptr, sizeof(T) + extra_size);
        }
    }

    // Get total allocated memory size
    static std::size_t get_total_allocated()
    {
        return total_allocated.load(std::memory_order_relaxed);
    }

    // Get current allocated memory size
    static std::size_t get_current_allocated()
    {
        return current_allocated.load(std::memory_order_relaxed);
    }

  public:
    static inline std::atomic<std::size_t> total_allocated{0};
    static inline std::atomic<std::size_t> current_allocated{0};
};