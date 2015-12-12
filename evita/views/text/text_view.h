// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_TEXT_VIEW_H_
#define EVITA_VIEWS_TEXT_TEXT_VIEW_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/views/text/render_selection.h"

namespace base {
class Time;
}

namespace ui {
class CaretOwner;
}

namespace views {

namespace rendering {
class LayoutBlockFlow;
class ScreenTextBlock;
}

class LayoutView;
class LayoutViewBuilder;
class TextViewCaret;

//////////////////////////////////////////////////////////////////////
//
// TextView
//
class TextView final {
 public:
  using LayoutBlockFlow = rendering::LayoutBlockFlow;
  using ScreenTextBlock = rendering::ScreenTextBlock;
  using TextSelectionModel = rendering::TextSelectionModel;
  using TextSelection = rendering::TextSelection;

  TextView(text::Buffer* buffer, ui::CaretOwner* caret_owner);
  ~TextView();

  text::Buffer* buffer() const { return buffer_; }

  void DidChangeStyle(text::Posn offset, size_t length);
  void DidDeleteAt(text::Posn offset, size_t length);
  void DidHide();
  void DidInsertAt(text::Posn offset, size_t length);
  void DidRecreateCanvas();
  // Returns end of line offset containing |text_offset|.
  text::Posn EndOfLine(text::Posn text_offset) const;
  text::Posn GetStart();
  text::Posn GetEnd();
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  text::Posn GetVisibleEnd();
  void Format(text::Posn text_offset);
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  gfx::RectF HitTestTextPosition(text::Posn text_offset) const;
  void MakeSelectionVisible();
  text::Posn MapPointToPosition(gfx::PointF point);
  text::Posn MapPointXToOffset(text::Posn text_offset, float point_x) const;
  void Paint(gfx::Canvas* canvas, base::Time now);
  bool ScrollDown();
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  // Returns start of line offset containing |text_offset|.
  text::Posn StartOfLine(text::Posn text_offset) const;
  void Update(const TextSelectionModel& selection);

 private:
  bool IsPositionFullyVisible(text::Posn text_offset) const;
  void ScrollToPosition(text::Posn offset);

  gfx::RectF bounds_;
  text::Buffer* const buffer_;
  std::unique_ptr<TextViewCaret> caret_;
  text::Posn caret_offset_;
  // The last |LayoutView| in screen.
  scoped_refptr<LayoutView> last_layout_view_;
  std::unique_ptr<LayoutBlockFlow> layout_block_flow_;
  std::unique_ptr<LayoutViewBuilder> layout_view_builder_;
  scoped_refptr<LayoutView> layout_view_;
  std::unique_ptr<ScreenTextBlock> screen_text_block_;

  DISALLOW_COPY_AND_ASSIGN(TextView);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_TEXT_VIEW_H_
