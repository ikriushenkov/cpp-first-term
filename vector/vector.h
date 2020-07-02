#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector();                               // O(1) nothrow
    vector(vector const&);                  // O(N) strong
    vector& operator=(vector const& other); // O(N) strong

    ~vector();                              // O(N) nothrow

    T& operator[](size_t i);                // O(1) nothrow
    T const& operator[](size_t i) const;    // O(1) nothrow

    T* data();                              // O(1) nothrow
    T const* data() const;                  // O(1) nothrow
    size_t size() const;                    // O(1) nothrow

    T& front();                             // O(1) nothrow
    T const& front() const;                 // O(1) nothrow

    T& back();                              // O(1) nothrow
    T const& back() const;                  // O(1) nothrow
    void push_back(T const&);               // O(1)* strong
    void pop_back();                        // O(1) nothrow

    bool empty() const;                     // O(1) nothrow

    size_t capacity() const;                // O(1) nothrow
    void reserve(size_t);                   // O(N) strong
    void shrink_to_fit();                   // O(N) strong

    void clear();                           // O(N) nothrow

    void swap(vector&);                     // O(1) nothrow

    iterator begin();                       // O(1) nothrow
    iterator end();                         // O(1) nothrow

    const_iterator begin() const;           // O(1) nothrow
    const_iterator end() const;             // O(1) nothrow

    iterator insert(const_iterator pos, T const&); // O(N) weak

    iterator erase(const_iterator pos);     // O(N) weak

    iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
    void increase_capacity();
    void copy(T* from, T* to, size_t size);
    void new_buffer(size_t new_capacity);
    void destroy_data(T* data, size_t capacity);

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};



template <typename T>
vector <T>::vector() : data_(nullptr), size_(0), capacity_(0) {}

template <typename T>
vector <T>::vector(vector const& other) : data_(nullptr), size_(0), capacity_(0) {
    reserve(other.size_);
    size_ = other.size_;
    copy(other.data_, data_, size_);
}

template <typename T>
vector<T>& vector<T>::operator=(vector const& other) {
    vector <T> tmp(other);
    swap(tmp);
    return *this;
}


template <typename T>
vector<T>::~vector() {
    clear();
    operator delete(data_);
}

template <typename T>
T& vector<T>::operator[](size_t i) {
    return data_[i];
}

template<typename T>
T const& vector<T>::operator[](size_t i) const {
    return data_[i];
}

template <typename T>
T* vector <T>::data() {
    return data_;
}

template<typename T>
T const* vector<T>::data() const {
    return data_;
}

template <typename T>
size_t vector<T>::size() const {
    return size_;
}

template <typename T>
T& vector<T>::front() {
    return *data_;
}

template<typename T>
T const& vector<T>::front() const {
    return *data_;
}

template <typename T>
T& vector<T>::back() {
    return data_[size_ - 1];
}

template<typename T>
T const& vector<T>::back() const {
    return data_[size_ - 1];
}

template <typename T>
void vector<T>::push_back(T const& val) {
    if (size_ == capacity_) {
        T new_val = val;
        increase_capacity();
        new (data_ + size_) T(new_val);
    } else {
        new (data_ + size_) T(val);
    }
    ++size_;
}

template <typename T>
void vector<T>::pop_back() {
    data_[--size_].~T();
}

template <typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}


template <typename T>
size_t vector <T>::capacity() const {
    return capacity_;
}

template <typename T>
void vector<T>::reserve(size_t new_capacity) {
    if (new_capacity > capacity_) {
        new_buffer(new_capacity);
    }
}

template <typename T>
void vector<T>::shrink_to_fit() {
    if (size_ == 0) {
        operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
        return;
    }
    if (size_ != capacity_) {
        new_buffer(size_);
    }
}

template <typename T>
void vector<T>::clear() {
    for (size_t i = size_; i > 0; --i) {
        data_[i - 1].~T();
    }
    size_ = 0;
}

template <typename T>
void vector<T>::swap(vector& other) {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
}

template <typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template <typename T>
typename vector<T>::iterator vector<T>::end() {
    return (data_ + size_);
}

template <typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return (data_ + size_);
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(const_iterator index, T const& val) {
    ptrdiff_t it = index - data_;
    push_back(val);
    for (size_t i = size_ - 1; i > it; --i) {
        std::swap(data_[i], data_[i - 1]);
    }
    return (data_ + it);
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator index) {
    return erase(index, index + 1);
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator left, const_iterator right) {
    ptrdiff_t it = right - data_;
    ptrdiff_t d = right - left;
    for (size_t i = it; i < size_; ++i) {
        std::swap(data_[i - d], data_[i]);
    }
    for (size_t i = size_; i > (size_ - d); --i) {
        data_[i - 1].~T();
    }
    size_ -= d;
    return (data_ + (left - data_));
}

template <typename T>
void vector<T>::new_buffer(size_t new_capacity) {
    T* new_data = static_cast<T*>(operator new(new_capacity * sizeof(T)));
    copy(data_, new_data, size_);
    std::swap(data_, new_data);
    destroy_data(new_data, size_);
    capacity_ = new_capacity;
}

template <typename T>
void vector<T>::copy(T* from, T* to, size_t size) {
    size_t i = 0;
    try {
        for(; i < size; ++i) {
            new (to + i) T(from[i]);
        }
    } catch (...) {
        destroy_data(to, i);
        throw;
    }
}

template <typename T>
void vector<T>::increase_capacity() {
    capacity_ == 0 ? reserve(1) : reserve(capacity_ << 1);
}

template <typename T>
void vector<T>::destroy_data(T* data, size_t capacity) {
    for (size_t i = capacity; i > 0; --i) {
        data[i - 1].~T();
    }
    operator delete(data);
}

#endif // VECTOR_H
