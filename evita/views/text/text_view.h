// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_TEXT_VIEW_H_
#define EVITA_VIEWS_TEXT_TEXT_VIEW_H_

#include <memory>

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
class Cell;
class ScreenTextBlock;
class TextBlock;
class TextFormatter;
class TextBlock;
class TextLine;
};

//////////////////////////////////////////////////////////////////////
//
// TextView
//
class TextView final {
 public:
  using Cell = rendering::Cell;

  typedef rendering::ScreenTextBlock ScreenTextBlock;
  typedef rendering::TextSelectionModel TextSelectionModel;
  typedef rendering::TextBlock TextBlock;
  typedef rendering::TextLine TextLine;

  TextView(text::Buffer* buffer, ui::CaretOwner* caret_owner);
  ~TextView();

  text::Buffer* buffer() const { return buffer_; }
  float zoom() const { return zoom_; }

  void DidChangeStyle(Posn offset, size_t length);
  void DidDeleteAt(Posn offset, size_t length);
  void DidHide();
  void DidInsertAt(Posn offset, size_t length);
  void DidRecreateCanvas();
  // Returns end of line offset containing |text_offset|.
  text::Posn EndOfLine(text::Posn text_offset) const;
  Posn GetStart();
  Posn GetEnd();
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  Posn GetVisibleEnd();
  void Format(text::Posn text_offset);
  gfx::RectF HitTestTextPosition(text::Posn text_offset) const;
  void MakeSelectionVisible();
  text::Posn MapPointToPosition(gfx::PointF point);
  text::Posn MapPointXToOffset(text::Posn text_offset, float point_x) const;
  bool ScrollDown();
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  // Returns start of line offset containing |text_offset|.
  text::Posn StartOfLine(text::Posn text_offset) const;
  void UpdateAndPaint(gfx::Canvas* canvas,
                      const TextSelectionModel& selection,
                      base::Time now);

 private:
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  bool IsPositionFullyVisible(text::Posn text_offset) const;
  void Paint(gfx::Canvas* canvas,
             const TextSelectionModel& selection,
             base::Time now);
  void PaintRuler(gfx::Canvas* canvas);
  void ScrollToPosition(text::Posn offset);
  bool ShouldPaint() const;
  bool ShouldFormat() const;

  gfx::RectF bounds_;
  text::Buffer* const buffer_;
  text::Posn caret_offset_;
  int format_counter_;
  std::unique_ptr<ScreenTextBlock> screen_text_block_;
  bool should_paint_;
  std::unique_ptr<TextBlock> text_block_;
  float zoom_;

  DISALLOW_COPY_AND_ASSIGN(TextView);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_TEXT_VIEW_H_
