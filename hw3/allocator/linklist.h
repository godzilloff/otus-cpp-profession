#ifndef ALLOCATOR_LINKED_LIST_H
#define ALLOCATOR_LINKED_LIST_H

#include "aalloc.h"

template <typename T>
struct ListNode {
    T value;
    ListNode* next;

    template <typename... Args>
    ListNode(Args&&... args) : value(std::forward<Args>(args)...), next(nullptr) {}
};

template <typename T, typename Allocator = std::allocator<ListNode<T>>>
class SimpleList {
private:
    using node_type = ListNode<T>;
    using node_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<node_type>;
    using alloc_traits = std::allocator_traits<node_allocator_type>;

    node_type* head_;
    node_allocator_type alloc_;
    size_t size_;

public:
    class iterator {
        node_type* current_;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(node_type* ptr) : current_(ptr) {}

        reference operator*() const { return current_->value; }
        pointer operator->() { return &current_->value; }

        iterator& operator++() {
            current_ = current_->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    explicit SimpleList(const Allocator& a = Allocator{})
        : head_(nullptr), alloc_(a), size_(0) {}

    ~SimpleList() {
        clear();
    }

    SimpleList(const SimpleList&) = delete;
    SimpleList& operator=(const SimpleList&) = delete;

    void push_front(const T& value) {
        node_type* new_node = alloc_traits::allocate(alloc_, 1);
        try {
            alloc_traits::construct(alloc_, new_node, value);
        } catch (...) {
            alloc_traits::deallocate(alloc_, new_node, 1);
            throw;
        }
        new_node->next = head_;
        head_ = new_node;
        ++size_;
    }

    void push_front(T&& value) {
        node_type* new_node = alloc_traits::allocate(alloc_, 1);
        try {
            alloc_traits::construct(alloc_, new_node, std::move(value));
        } catch (...) {
            alloc_traits::deallocate(alloc_, new_node, 1);
            throw;
        }
        new_node->next = head_;
        head_ = new_node;
        ++size_;
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        node_type* new_node = alloc_traits::allocate(alloc_, 1);
        try {
            alloc_traits::construct(alloc_, new_node, std::forward<Args>(args)...);
        } catch (...) {
            alloc_traits::deallocate(alloc_, new_node, 1);
            throw;
        }
        new_node->next = head_;
        head_ = new_node;
        ++size_;
    }

    iterator begin() const { return iterator(head_); }
    iterator end() const { return iterator(nullptr); }

    bool empty() const { return head_ == nullptr; }
    size_t size() const { return size_; }

    void clear() {
        while (head_) {
            node_type* tmp = head_;
            head_ = head_->next;
            alloc_traits::destroy(alloc_, tmp);
            alloc_traits::deallocate(alloc_, tmp, 1);
            --size_;
        }
    }

    void print() const {
        for (const auto& val : *this) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
};

#endif //ALLOCATOR_LINKED_LIST_H
