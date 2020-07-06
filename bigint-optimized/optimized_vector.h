#ifndef OPTIMIZED_VECTOR_H
#define OPTIMIZED_VECTOR_H

#include <utility>
#include <vector>
#include <cstdint>

struct optimized_vector {
    optimized_vector() {
        small_object = true;
        empty = true;
        vector.small = 0;
    }

    explicit optimized_vector(size_t size, uint32_t val = 0) {
        small_object = (size == 1);
        empty = false;
        if (small_object) {
            vector.small = val;
        } else {
            vector.big = new vector_with_count(size, val);
        }
    }

    optimized_vector(optimized_vector const &other) {
        small_object = other.small_object;
        empty = other.empty;
        if (!empty) {
            if (small_object) {
                vector.small = other.vector.small;
            } else {
                vector.big = other.vector.big;
                ++vector.big->count;
            }
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
                convert_to_small();
            }
        }
        small_object = other.small_object;
        empty = other.empty;
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
            return vector.small;
        } else {
            prep_for_changes();
            return vector.big->data_[i];
        }
    }

    uint32_t const& operator[](size_t i) const {
        if (small_object) {
            return vector.small;
        } else {
            return vector.big->data_[i];
        }
    }

    void push_back(uint32_t const val) {
        if (empty) {
            empty = false;
            vector.small = val;
        } else {
            if (small_object) {
                convert_to_big();
                vector.big->data_.push_back(val);
            } else {
                prep_for_changes();
                vector.big->data_.push_back(val);
            }
        }
    }

    size_t size() const {
        if (empty) {
            return 0;
        } else {
            return small_object ? 1 : vector.big->data_.size();
        }
    }

    void resize(size_t new_size, uint32_t val = 0) {
        if (new_size <= 1) {
            if (empty) {
                vector.small = val;
            }
        } else {
            if (small_object) {
                convert_to_big();
            }
            vector.big->data_.resize(new_size, val);
        }
    }

    uint32_t& back() {
        if (small_object) {
            return vector.small;
        } else {
            prep_for_changes();
            return vector.big->data_.back();
        }
    }

    uint32_t const& back() const {
        return small_object ? vector.small : vector.big->data_.back();
    }

    void pop_back() {
        if (small_object) {
            empty = true;
        } else {
            prep_for_changes();
            vector.big->data_.pop_back();
            if (vector.big->data_.size() == 1) {
                convert_to_small();
            }
        }
    }

    friend bool operator==(optimized_vector const& a, optimized_vector const& b) {
        if (a.small_object ^ b.small_object) {
            return false;
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

    bool small_object;
    bool empty;

    union {
        uint32_t small;
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
        uint32_t temp = vector.small;
        vector.big = new vector_with_count();
        vector.big->data_.push_back(temp);
    }

    void convert_to_small() {
        small_object = true;
        uint32_t temp = vector.big->data_.back();
        vector.small = temp;
    }

};

#endif //OPTIMIZED_VECTOR_H
