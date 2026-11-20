/**
 * @file Heap.h
 * @brief Binary heap 
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Before>
class Heap {
    std::vector<T> data_;
    Before before_;

    // Index helpers. parent() is only valid for i > 0.
    static std::size_t parent(std::size_t i) { return (i - 1) / 2; }
    static std::size_t left(std::size_t i) { return 2 * i + 1; }
    static std::size_t right(std::size_t i) { return 2 * i + 2; }

    void siftUp(std::size_t i);     
    void siftDown(std::size_t i);   

public:
    Heap() = default;
    explicit Heap(Before before) : before_(before) {}

    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }

    void push(const T& x);          // add an element
    const T& top() const;           // element that comes out next; throws if empty
    T pop();                        // remove and return it; throws if empty

    // Change every element (e.g. "everyone worsens"), then restore the heap property.
    void updateAll(const std::function<void(T&)>& change);
    void rebuild();                 // restore the heap property for the whole vector

    // Read-only view in heap (array) order. Index i is drawn at tree level floor(log2(i+1)),
    // so the GUI can draw the heap as a tree straight from this.
    const std::vector<T>& items() const { return data_; }

    bool isValid() const;           // debugging: does every element satisfy the heap property?
};

// ============================================================================

template <typename T, typename Before>
void Heap<T, Before>::push(const T& x) {
    data_.push_back(x);              // new element starts as the last leaf
    siftUp(data_.size() - 1);        // then climbs to where it belongs
}

template <typename T, typename Before>
const T& Heap<T, Before>::top() const {
    if (data_.empty()) {
        throw std::out_of_range("Heap::top on empty heap");
    }
    return data_[0];                 // the root comes out first
}

template <typename T, typename Before>
T Heap<T, Before>::pop() {
    if (data_.empty()) {
        throw std::out_of_range("Heap::pop on empty heap");
    }
    T root = data_[0];               // save the answer before overwriting it
    data_[0] = data_.back();         // last leaf takes the root's place
    data_.pop_back();
    if (!data_.empty()) {
        siftDown(0);                 // push it back down to where it belongs
    }
    return root;
}

template <typename T, typename Before>
void Heap<T, Before>::siftUp(std::size_t i) {
    // i > 0 is checked first: parent(0) would wrap around (size_t is unsigned).
    while (i > 0 && before_(data_[i], data_[parent(i)])) {
        std::swap(data_[i], data_[parent(i)]);
        i = parent(i);
    }
}

template <typename T, typename Before>
void Heap<T, Before>::siftDown(std::size_t i) {
    std::size_t best = i;

    // Does the left child exist, and should it come out before the current best?
    if (left(i) < size() && before_(data_[left(i)], data_[best])) {
        best = left(i);
    }
    // Same for the right child, compared against the winner so far
    if (right(i) < size() && before_(data_[right(i)], data_[best])) {
        best = right(i);
    }

    if (best == i) {
        return;                      // both children come after i: heap property holds here
    }

    std::swap(data_[i], data_[best]);
    siftDown(best);                  // keep pushing the moved element down
}

template <typename T, typename Before>
void Heap<T, Before>::rebuild() {
    for (std::size_t i = data_.size() / 2; i > 0; --i) {
        siftDown(i - 1);
    }
}

template <typename T, typename Before>
void Heap<T, Before>::updateAll(const std::function<void(T&)>& change) {
    for (T& x : data_) {
        change(x);                   
    }
    rebuild();                       
}

template <typename T, typename Before>
bool Heap<T, Before>::isValid() const {
    for (std::size_t i = 1; i < data_.size(); ++i) {
        if (before_(data_[i], data_[parent(i)])) {
            return false;            
        }
    }
    return true;
}