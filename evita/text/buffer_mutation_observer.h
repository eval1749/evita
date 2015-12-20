// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_BUFFER_MUTATION_OBSERVER_H_
#define EVITA_TEXT_BUFFER_MUTATION_OBSERVER_H_

#include "base/basictypes.h"

namespace text {

class Buffer;
class Offset;
class OffsetDelta;
class StaticRange;

//////////////////////////////////////////////////////////////////////
//
// BufferMutationObserver
//
class BufferMutationObserver {
 public:
  virtual ~BufferMutationObserver();

  virtual void DidChangeStyle(const StaticRange& range);
  virtual void DidDeleteAt(const StaticRange& range);
  virtual void DidInsertBefore(const StaticRange& range);
  virtual void WillDeleteAt(const StaticRange& range);

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
