#ifndef HALE_VECTOR_H
#define HALE_VECTOR_H

#include <vector>

namespace hale {

template<typename T>
struct Vector
{
    std::vector<T> *v_;
    memi count_;

    // std::vector compatibility boilerplate
    Vector() : count_(0) {}
    Vector(const Vector &other) : v_(other.v_), count_(other.count_) {}

//    Vector &operator =(const Vector &other) {
//        if (v_ && other.v_) {
//            *v_ = (*other.v_);
//            count_ = other.count_;
//        }
//        return *this;
//    }

    T &operator [](memi index) {
        return (*v_)[index];
    }
};

template<typename T>
struct Segment
{
    T *begin;
    T *end;
};

template<typename T>
inline memi
vector_count(Vector<T> *vector)
{
    // return (*((memi*)vector-1))
    return vector->count_;
}

template<typename T>
inline memi
vector_count(Vector<T> &vector)
{
    // return (*((memi*)vector-1))
    return vector.count_;
}

template<typename T>
inline Vector<T> *
vector_allocate()
{
    return new Vector<T>();
}

template<typename T>
inline void
vector_init(Vector<T> *vector, memi reserve = 0)
{
    vector->v_ = new std::vector<T>();
    vector->v_->reserve(reserve);
    vector->count_ = 0;
}

template<typename T>
inline void
vector_release(Vector<T> *vector)
{
    hale_assert(vector);
}

template<typename T>
inline T *
vector_data(Vector<T> *vector)
{
    return &vector->v_->operator [](0);
}

template<typename T>
inline b32
equal(Vector<T> *a, Vector<T> *b)
{
    return (*a->v_) == (*b->v_);
}

template<typename T>
inline void
vector_resize(Vector<T> *vector, memi size)
{
    vector->v_->resize(size);
    vector->count_ = size;
}

template<typename T>
inline void
vector_clear(Vector<T> *vector)
{
    vector->v_->clear();
    vector->count_ = 0;
}

template<typename T>
inline void
vector_push(Vector<T> *vector, const T &item)
{
    vector->v_->push_back(item);
    vector->count_++;
}

template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, Vector<T> *other)
{
    vector->v_.insert(vector->v_.begin() + offset, other->v_.begin(), other->v_.end());
    vector->count += other->count;
}

template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, T *other, memi other_count)
{
    vector->v_->insert(vector->v_->begin() + offset, other, other + other_count);
    vector->count_ += other_count;
}

template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, T *other_begin, T *other_end)
{
    hale_assert_debug(other_begin <= other_end);
    vector->v_->insert(vector->v_->begin() + offset, other_begin, other_end);
    vector->count_ += other_end - other_begin;
}

template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, memi count, const T &value)
{
    vector->v_->insert(vector->v_->begin() + offset, count, value);
    vector->count_ += count;
}

template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, memi count, T &value)
{
    vector->v_->insert(vector->v_->begin() + offset, count, value);
    vector->count_ += count;
}


template<typename T>
inline void
vector_insert(Vector<T> *vector, memi offset, memi count)
{
    // TODO: This is hack.
    vector->v_->insert(vector->v_->begin() + offset, count, T());
    vector->count_ += count;
}

template<typename T>
inline void
vector_append(Vector<T> *vector, T *other, memi count)
{
    vector_insert(vector, vector_count(vector), other, other + count);
}

template<typename T>
inline void
vector_remove(Vector<T> *vector, T* begin, memi count)
{
    auto it = vector->v_->begin() + (begin - vector_begin(vector));
    vector->v_->erase(it, it + count);
    vector->count_ -= count;
}

template<typename T>
inline void
vector_remove(Vector<T> *vector, T* begin, T* end)
{
    auto b = vector->v_->begin() + (memi)(begin - vector_begin(vector));
    auto e = vector->v_->begin() + (memi)(end   - vector_begin(vector));
    vector->v_->erase(b, e);
    vector->count_ -= end - begin;
}

template<typename T>
inline void
vector_copy(Vector<T> *destination, Vector<T> *source)
{
    destination->v_ = source->v_;
    destination->count_ = source->count;
}

template<typename T>
inline void
vector_copy(Vector<T> *destination, T *begin, T* end)
{
    hale_assert(begin <= end);
    vector_resize(destination, end - begin);
    platform.move_memory(vector_begin(destination), begin, (end - begin) * sizeof(T));
}

template<typename T>
inline T &
vector_last(Vector<T> *vector)
{
    return vector->v_->back();
}

template<typename T>
inline T &
vector_first(Vector<T> *vector)
{
    return vector->v_->front();
}

template<typename T>
inline T *
vector_begin(Vector<T> *vector)
{
    return &vector->v_->operator [](0);
}

template<typename T>
inline T *
vector_end(Vector<T> *vector)
{
    return vector_begin(vector) + vector_count(vector);
}

template<typename T>
inline T &
vector_at(Vector<T> *vector, memi index)
{
    return vector->v_->operator [](index);
}

} // namespace hale

#endif // HALE_VECTOR_H
