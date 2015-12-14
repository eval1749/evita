// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_SELECTION_H_
#define EVITA_TEXT_SELECTION_H_

#include <memory>

#include "base/macros.h"
#include "evita/css/style.h"

namespace text {

class Buffer;
class Offset;
class Range;
class SelectionChangeObserver;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection final {
 public:
  explicit Selection(const Range* range);
  ~Selection();

  Offset anchor_offset() const;
  Buffer* buffer() const;
  Offset end() const;
  Offset focus_offset() const;
  Range* range() const;
  Offset start() const;

  void AddObserver(SelectionChangeObserver* observer);
  bool IsStartActive() const;
  void RemoveObserver(SelectionChangeObserver* observer);
  void SetStartIsActive(bool new_start_is_active);

 private:
  class Model;

  std::unique_ptr<Model> model_;

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace text

#endif  // EVITA_TEXT_SELECTION_H_
