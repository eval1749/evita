// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_INTERVAL_SET_OBSERVER_H_
#define EVITA_TEXT_INTERVAL_SET_OBSERVER_H_

namespace text {

class Offset;

//////////////////////////////////////////////////////////////////////
//
// IntervalSetObserver
//
class IntervalSetObserver {
 public:
  virtual ~IntervalSetObserver();

  virtual void DidChangeInterval(Offset start, Offset end) = 0;

 protected:
  IntervalSetObserver();
};

}  // namespace text

#endif  // EVITA_TEXT_INTERVAL_SET_OBSERVER_H_
