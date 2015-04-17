// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_selection_h)
#define INCLUDE_evita_text_selection_h

#include <memory>

#include "base/macros.h"
#include "evita/css/style.h"
#include "evita/precomp.h"

namespace text {
class Buffer;
class Range;
class SelectionChangeObserver;

class Selection {
  private: class Model;

  private: std::unique_ptr<Model> model_;

  public: Selection(const Range* range);
  public: ~Selection();

  public: Posn anchor_offset() const;
  public: Buffer* buffer() const;
  public: Posn end() const;
  public: Posn focus_offset() const;
  public: Range* range() const;
  public: Posn start() const;

  public: void AddObserver(SelectionChangeObserver* observer);
  public: bool IsStartActive() const;
  public: void RemoveObserver(SelectionChangeObserver* observer);
  public: void SetStartIsActive(bool new_start_is_active);

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_selection_h)
