// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_BUFFER_MUTATION_OBSERVER_H_
#define EVITA_TEXT_BUFFER_MUTATION_OBSERVER_H_

#include "base/basictypes.h"
#include "evita/precomp.h"

namespace text {

class Buffer;

class BufferMutationObserver {
 public:
  virtual ~BufferMutationObserver();

  virtual void DidChangeStyle(Posn offset, size_t length);
  virtual void DidDeleteAt(Posn offset, size_t length);
  virtual void DidInsertAt(Posn offset, size_t length);
  virtual void DidInsertBefore(Posn offset, size_t length);
  virtual void WillDeleteAt(Posn offset, size_t length);

 protected:
  BufferMutationObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(BufferMutationObserver);
};

class BufferMutationObservee {
 public:
  virtual ~BufferMutationObservee();

  virtual void AddObserver(BufferMutationObserver* observer) = 0;
  virtual void RemoveObserver(BufferMutationObserver* observer) = 0;

  BufferMutationObservee();

 private:
  DISALLOW_COPY_AND_ASSIGN(BufferMutationObservee);
};

}  // namespace text

#endif  // EVITA_TEXT_BUFFER_MUTATION_OBSERVER_H_
