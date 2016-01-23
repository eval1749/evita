// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ZONE_ALLOCATOR_H_
#define EVITA_BASE_MEMORY_ZONE_ALLOCATOR_H_

#include <limits>

#include "evita/base/memory/zone.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// ZoneAllocator
// A class for using |Zone| allocate with C++ |std| containers.
//
template <typename T>
class ZoneAllocator {
 public:
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  template <class O>
  struct rebind {
    typedef ZoneAllocator<O> other;
  };

  explicit ZoneAllocator(Zone* zone) throw() : zone_(zone) {}
  explicit ZoneAllocator(const ZoneAllocator& other) throw()
      : zone_(other.zone_) {}
  template <typename U>
  ZoneAllocator(const ZoneAllocator<U>& other) throw()
      : zone_(other.zone_) {}
  template <typename U>
  friend class ZoneAllocator;

  ZoneAllocator() = delete;
  ~ZoneAllocator() = default;

  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const { return &x; }

  Zone* zone() const { return zone_; }

  pointer allocate(size_type n, const void* hint = 0) {
    return static_cast<pointer>(zone_->AllocateObjects<value_type>(n));
  }
  void deallocate(pointer p, size_type) { /* noop for Zones */
  }

  size_type max_size() const throw() {
    return std::numeric_limits<int>::max() / sizeof(value_type);
  }
  void construct(pointer p, const T& val) {
    new (static_cast<void*>(p)) T(val);
  }
  void destroy(pointer p) { p->~T(); }

  bool operator==(ZoneAllocator const& other) const {
    return zone_ == other.zone_;
  }
  bool operator!=(ZoneAllocator const& other) const {
    return zone_ != other.zone_;
  }

 private:
  Zone* zone_;
};

}  // namespace evita

#endif  // EVITA_BASE_MEMORY_ZONE_ALLOCATOR_H_
