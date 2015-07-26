// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_EITHER_H_
#define COMMON_EITHER_H_

namespace common {

template <typename Left, typename Right>
struct Either {
  Left left;
  Right right;

  Either(Left left, Right right) : left(left), right(right) {}
};

template <typename Left, typename Right>
Either<Left, Right> make_either(const Left& left, const Right& right) {
  return Either<Left, Right>(left, right);
}

}  // namespace common

#endif  // COMMON_EITHER_H_
