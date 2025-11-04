#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {

template <typename T>
class vector {
 private:
  T *data_ = nullptr;
  size_t sz_ = 0;
  size_t cap_ = 0;

  static T *raw_alloc(size_t n) { return (T *)::operator new(n * sizeof(T)); }
  static void raw_free(T *p) { ::operator delete(p); }

  void ensure_capacity(size_t need) {
    if (need <= cap_) return;
    size_t ncap = cap_ ? cap_ : 1;
    while (ncap < need) ncap <<= 1;
    T *nd = raw_alloc(ncap);
    size_t i = 0;
    try {
      for (; i < sz_; ++i) new (nd + i) T(data_[i]);
    } catch (...) {
      for (size_t j = 0; j < i; ++j) nd[j].~T();
      raw_free(nd);
      throw;
    }
    for (size_t j = 0; j < sz_; ++j) data_[j].~T();
    raw_free(data_);
    data_ = nd;
    cap_ = ncap;
  }

 public:
  class const_iterator;
  class iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

   private:
    const vector *owner = nullptr;
    size_t idx = 0;
    friend class vector;
    friend class const_iterator;

   public:
    iterator() = default;
    iterator(const vector *o, size_t i) : owner(o), idx(i) {}

    iterator operator+(const int &n) const { return iterator(owner, idx + n); }
    iterator operator-(const int &n) const { return iterator(owner, idx - n); }
    int operator-(const iterator &rhs) const {
      if (owner != rhs.owner) throw invalid_iterator();
      return static_cast<int>(idx) - static_cast<int>(rhs.idx);
    }
    iterator &operator+=(const int &n) {
      idx += n;
      return *this;
    }
    iterator &operator-=(const int &n) {
      idx -= n;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    iterator &operator++() {
      ++idx;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }
    iterator &operator--() {
      --idx;
      return *this;
    }
    T &operator*() const {
      if (owner == nullptr || idx >= owner->sz_) throw invalid_iterator();
      return const_cast<T &>(owner->data_[idx]);
    }
    bool operator==(const iterator &rhs) const {
      return owner == rhs.owner && idx == rhs.idx;
    }
    bool operator==(const const_iterator &rhs) const {
      return owner == rhs.owner && idx == rhs.idx;
    }
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
    bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
  };

  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T *;
    using reference = const T &;
    using iterator_category = std::output_iterator_tag;

   private:
    const vector *owner = nullptr;
    size_t idx = 0;
    friend class vector;
    friend class iterator;

   public:
    const_iterator() = default;
    const_iterator(const vector *o, size_t i) : owner(o), idx(i) {}
    const_iterator(const iterator &it) : owner(it.owner), idx(it.idx) {}

    const_iterator operator+(const int &n) const {
      return const_iterator(owner, idx + n);
    }
    const_iterator operator-(const int &n) const {
      return const_iterator(owner, idx - n);
    }
    int operator-(const const_iterator &rhs) const {
      if (owner != rhs.owner) throw invalid_iterator();
      return static_cast<int>(idx) - static_cast<int>(rhs.idx);
    }
    const_iterator &operator+=(const int &n) {
      idx += n;
      return *this;
    }
    const_iterator &operator-=(const int &n) {
      idx -= n;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    const_iterator &operator++() {
      ++idx;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }
    const_iterator &operator--() {
      --idx;
      return *this;
    }
    const T &operator*() const {
      if (owner == nullptr || idx >= owner->sz_) throw invalid_iterator();
      return owner->data_[idx];
    }
    bool operator==(const const_iterator &rhs) const {
      return owner == rhs.owner && idx == rhs.idx;
    }
    bool operator==(const iterator &rhs) const {
      return owner == rhs.owner && idx == rhs.idx;
    }
    bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
  };

  vector() = default;
  vector(const vector &other) : data_(nullptr), sz_(0), cap_(0) {
    if (other.sz_) {
      data_ = raw_alloc(other.sz_);
      cap_ = other.sz_;
      size_t i = 0;
      try {
        for (; i < other.sz_; ++i) new (data_ + i) T(other.data_[i]);
      } catch (...) {
        for (size_t j = 0; j < i; ++j) data_[j].~T();
        raw_free(data_);
        data_ = nullptr;
        cap_ = 0;
        throw;
      }
      sz_ = other.sz_;
    }
  }
  ~vector() {
    clear();
    if (data_) raw_free(data_);
    data_ = nullptr;
    cap_ = 0;
  }
  vector &operator=(const vector &other) {
    if (this == &other) return *this;
    vector tmp(other);
    swap(tmp);
    return *this;
  }

  void swap(vector &rhs) {
    T *td = data_;
    data_ = rhs.data_;
    rhs.data_ = td;
    size_t ts = sz_;
    sz_ = rhs.sz_;
    rhs.sz_ = ts;
    size_t tc = cap_;
    cap_ = rhs.cap_;
    rhs.cap_ = tc;
  }

  T &at(const size_t &pos) {
    if (pos >= sz_) throw index_out_of_bound();
    return data_[pos];
  }
  const T &at(const size_t &pos) const {
    if (pos >= sz_) throw index_out_of_bound();
    return data_[pos];
  }

  T &operator[](const size_t &pos) {
    if (pos >= sz_) throw index_out_of_bound();
    return data_[pos];
  }
  const T &operator[](const size_t &pos) const {
    if (pos >= sz_) throw index_out_of_bound();
    return data_[pos];
  }

  const T &front() const {
    if (sz_ == 0) throw container_is_empty();
    return data_[0];
  }
  const T &back() const {
    if (sz_ == 0) throw container_is_empty();
    return data_[sz_ - 1];
  }

  iterator begin() { return iterator(this, 0); }
  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator cbegin() const { return const_iterator(this, 0); }

  iterator end() { return iterator(this, sz_); }
  const_iterator end() const { return const_iterator(this, sz_); }
  const_iterator cend() const { return const_iterator(this, sz_); }

  bool empty() const { return sz_ == 0; }
  size_t size() const { return sz_; }

  void clear() {
    for (size_t i = 0; i < sz_; ++i) data_[i].~T();
    sz_ = 0;
  }

  iterator insert(iterator pos, const T &value) {
    if (pos.owner != this) throw invalid_iterator();
    return insert(pos.idx, value);
  }

  iterator insert(const size_t &ind, const T &value) {
    if (ind > sz_) throw index_out_of_bound();
    ensure_capacity(sz_ + 1);
    if (ind == sz_) {
      new (data_ + sz_) T(value);
      ++sz_;
      return iterator(this, ind);
    }
    new (data_ + sz_) T(data_[sz_ - 1]);
    for (size_t i = sz_ - 1; i > ind; --i) {
      data_[i] = data_[i - 1];
    }
    data_[ind] = value;
    ++sz_;
    return iterator(this, ind);
  }

  iterator erase(iterator pos) {
    if (pos.owner != this || pos.idx >= sz_) throw invalid_iterator();
    size_t ind = pos.idx;
    if (ind == sz_ - 1) {
      data_[sz_ - 1].~T();
      --sz_;
      return end();
    }
    for (size_t i = ind; i + 1 < sz_; ++i) {
      data_[i] = data_[i + 1];
    }
    data_[sz_ - 1].~T();
    --sz_;
    return iterator(this, ind);
  }

  iterator erase(const size_t &ind) {
    if (ind >= sz_) throw index_out_of_bound();
    return erase(iterator(this, ind));
  }

  void push_back(const T &value) {
    ensure_capacity(sz_ + 1);
    new (data_ + sz_) T(value);
    ++sz_;
  }

  void pop_back() {
    if (sz_ == 0) throw container_is_empty();
    --sz_;
    data_[sz_].~T();
  }
};

}  // namespace sjtu

#endif
