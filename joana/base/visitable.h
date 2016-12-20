// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_VISITABLE_H_
#define JOANA_BASE_VISITABLE_H_

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// Derived class of |Visitable| implements visitor patter.
//
template <class Visitor>
class Visitable {
 public:
  virtual void Accept(Visitor* visitor) = 0;

 protected:
  Visitable() = default;
  ~Visitable() = default;
};

}  // namespace joana

#endif  // JOANA_BASE_VISITABLE_H_
