#ifndef OPTIMIZED_VECTOR_H
#define OPTIMIZED_VECTOR_H

#include <utility>
#include <vector>
#include <cstdint>

struct optimized_vector {
    optimized_vector() : small_object(true), vector({}) {}

    explicit optimized_vector(size_t size, uint32_t val = 0) : small_object(size <= 1), vector({}) {
        if (small_object) {
            vector.small = small_vector(size, val);
        } else {
            vector.big = new vector_with_count(size, val);
        }
    }

    optimized_vector(optimized_vector const &other) : small_object(other.small_object), vector({}) {
        if (small_object) {
            vector.small = other.vector.small;
        } else {
            vector.big = other.vector.big;
            ++vector.big->count;
        }
    }

    ~optimized_vector() {
        if (!small_object) {
            delete_one();
        }
    }

    optimized_vector& operator=(optimized_vector const &other) {
        if (small_object ^ other.small_object) {
            if (small_object) {
                convert_to_big();
            } else {
                prep_for_changes();
                vector.big->data_.clear();
                for (size_t i = 0; i < other.vector.small.size_; ++i) {
                    vector.big->data_.push_back(other.vector.small.data_[i]);
                }
                vector.big->count = 1;
                return *this;
            }
        }
        small_object = other.small_object;
        if (small_object) {
            vector.small = other.vector.small;
        } else {
            if (vector.big != other.vector.big) {
                delete_one();
                vector.big = other.vector.big;
                ++vector.big->count;
            }
        }
        return *this;
    }

    uint32_t& operator[](size_t i) {
        if (small_object) {
            return vector.small.data_[i];
        } else {
            prep_for_changes();
            return vector.big->data_[i];
        }
    }

    uint32_t const& operator[](size_t i) const {
        if (small_object) {
            return vector.small.data_[i];
        } else {
            return vector.big->data_[i];
        }
    }

    void push_back(uint32_t const val) {
        if (small_object) {
            if (vector.small.size_ == small_vector::SIZE) {
                convert_to_big();
                vector.big->data_.push_back(val);
            } else {
                vector.small.data_[vector.small.size_++] = val;
            }
        } else {
            prep_for_changes();
            vector.big->data_.push_back(val);
        }
    }

    size_t size() const {
        return small_object ? vector.small.size_ : vector.big->data_.size();
    }

    void resize(size_t new_size, uint32_t val = 0) {
        if (small_object) {
            if (new_size <= small_vector::SIZE) {
                while (vector.small.size_ < new_size) {
                    vector.small.data_[vector.small.size_++] = val;
                }
                return;
            } else {
                convert_to_big();
            }
        }
        vector.big->data_.resize(new_size, val);
    }

    uint32_t& back() {
        if (small_object) {
            return vector.small.data_[vector.small.size_ - 1];
        } else {
            prep_for_changes();
            return vector.big->data_.back();
        }
    }

    uint32_t const& back() const {
        return small_object ? vector.small.data_[vector.small.size_ - 1] : vector.big->data_.back();
    }

    void pop_back() {
        if (small_object) {
            --vector.small.size_;
        } else {
            prep_for_changes();
            vector.big->data_.pop_back();
        }
    }

    friend bool operator==(optimized_vector const& a, optimized_vector const& b) {
        if (a.small_object ^ b.small_object) {
            if (a.small_object) {
                return b.vector.big->data_ == a.vector.small;
            } else {
                return a.vector.big->data_ == b.vector.small;
            }
        } else {
            if (a.small_object) {
                return a.vector.small == b.vector.small;
            } else {
                return (a.vector.big == b.vector.big) || (a.vector.big->data_ == b.vector.big->data_);
            }
        }
    }

private:
    struct vector_with_count {
        std::vector <uint32_t> data_;
        size_t count;

        vector_with_count() : data_(), count(1) {}
        vector_with_count(size_t size, uint32_t val) : data_(size, val), count(1) {}
        vector_with_count(std::vector <uint32_t> const &other) : data_(other), count(1) {}
    };

    struct small_vector {
        static constexpr size_t SIZE = (sizeof(vector_with_count) - sizeof(size_t)) / sizeof(uint32_t);
        uint32_t data_[SIZE];
        size_t size_;

        small_vector() : data_(), size_(0) {}
        small_vector(size_t size, uint32_t val) : data_(), size_(size) {
            for (size_t i = 0; i < size; ++i) {
                data_[i] = val;
            }
        }

        small_vector& operator=(small_vector const& other) {
            size_ = other.size_;
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
            return *this;
        }

        friend bool operator==(std::vector<uint32_t> const& a, small_vector const& b) {
            if (a.size() == b.size_) {
                for (size_t i = 0; i < a.size(); ++i) {
                    if (a[i] != b.data_[i]) {
                        return false;
                    }
                }
                return true;
            } else {
                return false;
            }
        }

        friend bool operator==(small_vector const& a, small_vector const& b) {
            if (a.size_ == b.size_) {
                for (size_t i = 0; i < a.size_; ++i) {
                    if (a.data_[i] != b.data_[i]) {
                        return false;
                    }
                }
                return true;
            } else {
                return false;
            }
        }
    };

    bool small_object;

    union {
        small_vector small;
        vector_with_count *big;
    } vector;

    void prep_for_changes() {
        if (vector.big->count != 1) {
            --vector.big->count;
            vector.big = new vector_with_count(vector.big->data_);
        }
    }

    void delete_one() const {
        --vector.big->count;
        if (vector.big->count == 0) {
            delete vector.big;
        }
    }

    void convert_to_big() {
        small_object = false;
        small_vector temp = vector.small;
        vector.big = new vector_with_count();
        for (size_t i = 0; i < small_vector::SIZE; ++i) {
            vector.big->data_.push_back(temp.data_[i]);
        }
    }

};

#endif //OPTIMIZED_VECTOR_H
