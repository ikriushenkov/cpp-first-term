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

	iterator insert(iterator pos, T const&); // O(N) weak
	iterator insert(const_iterator pos, T const&); // O(N) weak

	iterator erase(iterator pos);           // O(N) weak
	iterator erase(const_iterator pos);     // O(N) weak

	iterator erase(iterator first, iterator last); // O(N) weak
	iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
	void increase_capacity();

private:
	T* data_;
	size_t size_;
	size_t capacity_;
};

#endif // VECTOR_H

template <typename T>
vector <T>::vector() {
	size_ = 0;
	capacity_ = 0;
	data_ = nullptr;
}

template <typename T>
vector <T>::vector(vector const& other) {
	size_ = 0;
	capacity_ = 0;
	data_ = nullptr;
	reserve(other.size_);
	size_ = other.size_;
	for (size_t i = 0; i < size_; ++i) {
		new(data_ + i)T(other.data_[i]);
	}
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
		increase_capacity();
	}
	new (data_ + size_) T(val);
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
void vector<T>::reserve(size_t c) {
	if (c > capacity_) {
		T* other = static_cast<T*>(operator new(c * sizeof(T)));
		size_t i;
		try {
			for (i = 0; i < size_; ++i) {
				new (other + i) T(data_[i]);
			}

			std::swap(data_, other);
			for (i = capacity_; i > 0; --i) {
				other[i - 1].~T();
			}
			capacity_ = c;
		} catch (...) {
			for (; i > 0; --i) {
				other[i - 1].~T();
			}
			throw;
		}
	}
}

template <typename T>
void vector<T>::shrink_to_fit() {
	if (size_ == 0) {
		data_ = nullptr;
		capacity_ = 0;
		return;
	}
	if (size_ != capacity_) {
		T* other;
		other = static_cast<T*>(operator new(size_ * sizeof(T)));
		for (size_t i = 0; i < size_; ++i) {
			new(other + i) T(data_[i]);
		}
		std::swap(data_, other);
		for (size_t i = size_; i > 0; --i) {
			other[i - 1].~T();
		}
		capacity_ = size_;
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
T* vector<T>::begin() {
	return data_;
}

template <typename T>
T const* vector<T>::begin() const {
	return data_;
}

template <typename T>
T* vector<T>::end() {
	return (data_ + size_);
}

template <typename T>
T const* vector<T>::end() const {
	return (data_ + size_);
}

template <typename T>
T* vector<T>::insert(iterator index, T const& val) {
	return insert(static_cast<T const*>(index), val);
}

template <typename T>
T* vector<T>::insert(const_iterator index, T const& val) {
	size_t it = index - data_;
	if (size_ == capacity_) {
		increase_capacity();
	}
	if (size_ == 0) {
		new(data_)T(val);
	} else {
		new(data_ + size_)T(data_[size_ - 1]);
		for (size_t i = size_ - 1; i > it; --i) {
			data_[i] = data_[i - 1];
		}
		data_[it] = val;
	}
	++size_;
	return (data_ + it);
}

template <typename T>
T* vector<T>::erase(iterator index) {
	return erase(static_cast<T const*>(index));
}

template <typename T>
T* vector<T>::erase(const_iterator index) {
	return erase(index, index + 1);
}

template <typename T>
T* vector<T>::erase(iterator left, iterator right) {
	return erase(static_cast<T const*>(left), static_cast<T const*>(right));
}

template <typename T>
T* vector<T>::erase(const_iterator left, const_iterator right) {
	size_t it = right - data_;
	size_t d = right - left;
	for (size_t i = it; i < size_; ++i) {
		data_[i - d] = data_[i];
	}
	for (size_t i = size_; i > (size_ - d); --i) {
		data_[i - 1].~T();
	}
	size_ -= d;
	return (data_ + (left - data_));
}

template <typename T>
void vector<T>::increase_capacity() {
	if (capacity_ == 0) {
		reserve(1);
	} else {
		reserve(capacity_ << 1);
	}
}