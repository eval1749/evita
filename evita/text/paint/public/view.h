// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_PAINT_PUBLIC_VIEW_H_
#define EVITA_TEXT_PAINT_PUBLIC_VIEW_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/paint/public/caret.h"

namespace layout {
// TODO(eval1749): We should have paint version of RootInlineBox.
class RootInlineBox;
}

namespace paint {

class RootInlineBox;
class Ruler;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// View
//
class View final : public base::RefCounted<View> {
 public:
  View(int layout_version,
       const gfx::RectF& bounds,
       const std::vector<RootInlineBox*>& lines,
       scoped_refptr<Selection> selection,
       const gfx::ColorF& bgcolor,
       const Ruler& ruler,
       const Caret& caret);
  ~View();

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const gfx::RectF& bounds() const { return bounds_; }
  const Caret& caret() const { return *caret_; }
  const std::vector<RootInlineBox*>& lines() const { return lines_; }
  const Ruler& ruler() const { return *ruler_; }
  scoped_refptr<Selection> selection() const { return selection_; }
  int layout_version() const { return layout_version_; }

 private:
  const gfx::ColorF bgcolor_;
  const gfx::RectF bounds_;
  const std::unique_ptr<Caret> caret_;
  const int layout_version_;
  const std::vector<RootInlineBox*> lines_;
  const std::unique_ptr<Ruler> ruler_;
  const scoped_refptr<Selection> selection_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace paint

#endif  // EVITA_TEXT_PAINT_PUBLIC_VIEW_H_
