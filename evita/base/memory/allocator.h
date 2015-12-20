// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_MEMORY_ALLOCATOR_H_
#define EVITA_BASE_MEMORY_ALLOCATOR_H_

namespace memory {

#define DISALLOW_NEW()                      \
 public:                                    \
  void* operator new(size_t size) = delete; \
  void* operator new(size_t size, void* pointer) = delete;

#define STACK_ALLOCATED() DISALLOW_NEW()

}  // namespace memory

#endif  // EVITA_BASE_MEMORY_ALLOCATOR_H_
