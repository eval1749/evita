// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_buffer_mutation_observer_h)
#define INCLUDE_evita_text_buffer_mutation_observer_h

namespace text {

class Buffer;

class BufferMutationObserver {
  public: BufferMutationObserver();
  public: virtual ~BufferMutationObserver();

  public: virtual void DidDeleteAt(Posn offset, size_t length) = 0;
  public: virtual void DidInsertAt(Posn offset, size_t length) = 0;
  public: virtual void DidInsertBefore(Posn offset, size_t length) = 0;
  public: virtual void WillDeleteAt(Posn offset, size_t length);

  DISALLOW_COPY_AND_ASSIGN(BufferMutationObserver);
};

}   // text

#endif //!defined(INCLUDE_evita_text_buffer_mutation_observer_h)
