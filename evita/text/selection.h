// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_SELECTION_H_
#define EVITA_TEXT_SELECTION_H_

#include <memory>

#include "base/macros.h"
#include "evita/css/style.h"
#include "evita/precomp.h"

namespace text {
class Buffer;
class Range;
class SelectionChangeObserver;

class Selection final {
 public:
  explicit Selection(const Range* range);
  ~Selection();

  Posn anchor_offset() const;
  Buffer* buffer() const;
  Posn end() const;
  Posn focus_offset() const;
  Range* range() const;
  Posn start() const;

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
