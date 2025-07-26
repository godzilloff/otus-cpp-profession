#ifndef ALLOCATOR_LIB_H
#define ALLOCATOR_LIB_H

#include <iostream>
#include <map>
#include <cstddef>
#include <new>
#include <cstring>
#include <type_traits>

using TargetPair = std::pair<const int, int>;

template <typename T, std::size_t N = 10>
class FixedPoolAllocator {
private:
    static constexpr std::size_t max_elements = N;
    
    alignas(T) char pool_[max_elements * sizeof(T)];
    bool used_[max_elements];
    bool initialized_ = false;

    static constexpr bool is_target_type =
        std::is_same_v<T, TargetPair>;

public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;

    template <typename U>
    struct rebind {
        using other = FixedPoolAllocator<U>;
    };

    FixedPoolAllocator() noexcept {
        if (is_target_type) {
            std::memset(used_, 0, sizeof(used_));
            initialized_ = true;
        }
    }

    template <typename U>
    FixedPoolAllocator(const FixedPoolAllocator<U>&) noexcept {
        if (is_target_type) {
            std::memset(used_, 0, sizeof(used_));
            initialized_ = true;
        }
    }

    ~FixedPoolAllocator() = default;

    FixedPoolAllocator(const FixedPoolAllocator&) = default;
    FixedPoolAllocator& operator=(const FixedPoolAllocator&) = delete;

    pointer allocate(size_type n, const void* = 0) {
        if (n != 1) throw std::bad_alloc();

        if constexpr (is_target_type) {
            // for std::pair<const int, int> use pool
            if (!initialized_) throw std::bad_alloc();
            for (std::size_t i = 0; i < max_elements; ++i) {
                if (!used_[i]) {
                    used_[i] = true;
                    return reinterpret_cast<pointer>(pool_ + i * sizeof(T));
                }
            }
            throw std::bad_alloc();
        } else {
            // other type - stantart
            return static_cast<pointer>(::operator new(sizeof(T)));
        }
    }

    void deallocate(pointer p, size_type n) noexcept {
        if (!p || n != 1) return;

        if constexpr (is_target_type) {
            std::size_t index = (reinterpret_cast<char*>(p) - pool_) / sizeof(T);
            if (index < max_elements && used_[index]) {
                used_[index] = false;
            }
        } else {
            ::operator delete(p);
        }
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        if (p) p->~U();
    }

    template <typename U>
    bool operator==(const FixedPoolAllocator<U>&) const noexcept {
        return true;
    }

    template <typename U>
    bool operator!=(const FixedPoolAllocator<U>& other) const noexcept {
        return !(*this == other);
    }
};

#endif //ALLOCATOR_LIB_H
