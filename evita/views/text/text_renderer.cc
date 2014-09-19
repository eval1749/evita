// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_DIRTY  0
#define DEBUG_DISPBUF 0
#define DEBUG_FORMAT 0
#define DEBUG_RENDER 0
#include "evita/views/text/text_renderer.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "evita/dom/lock.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx_base.h"
#include "evita/text/buffer.h"
#include "evita/views/text/render_font.h"
#include "evita/views/text/render_font_set.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/screen_text_block.h"
#include "evita/views/text/text_formatter.h"

namespace views {
using namespace rendering;

//////////////////////////////////////////////////////////////////////
//
// TextRenderer
//
TextRenderer::TextRenderer(text::Buffer* buffer)
    : buffer_(buffer), format_counter_(0),
      screen_text_block_(new ScreenTextBlock()), should_render_(true),
      text_block_(new TextBlock(buffer)), zoom_(1.0f) {
}

TextRenderer::~TextRenderer() {
}

void TextRenderer::DidHide() {
  screen_text_block_->Reset();
}

void TextRenderer::DidRecreateCanvas() {
  screen_text_block_->Reset();
}

text::Posn TextRenderer::EndOfLine(text::Posn text_offset) const {
  return text_block_->EndOfLine(text_offset);
}

text::Posn TextRenderer::GetEnd() {
  return text_block_->GetEnd();
}

text::Posn TextRenderer::GetStart() {
  return text_block_->GetStart();
}

text::Posn TextRenderer::GetVisibleEnd() {
  return text_block_->GetVisibleEnd();
}

void TextRenderer::Format(text::Posn text_offset) {
  text_block_->Format(text_offset);
  should_render_ = true;
}

bool TextRenderer::FormatIfNeeded() {
  if (!text_block_->FormatIfNeeded())
    return format_counter_ != text_block_->format_counter();
  should_render_ = true;
  return true;
}

// Maps specified buffer position to window point and returns true. If
// specified buffer point isn't in window, this function returns false.
//
// A TextRenderer object must be formatted with the latest buffer.
//
gfx::RectF TextRenderer::HitTestTextPosition(text::Posn text_offset) const {
  return text_block_->HitTestTextPosition(text_offset);
}

bool TextRenderer::IsPositionFullyVisible(text::Posn offset) const {
  return text_block_->IsPositionFullyVisible(offset);
}

text::Posn TextRenderer::MapPointToPosition(gfx::PointF point) {
  return text_block_->MapPointToPosition(point);
}

text::Posn TextRenderer::MapPointXToOffset(text::Posn text_offset,
                                           float point_x) const {
  return text_block_->MapPointXToOffset(text_offset, point_x);
}

void TextRenderer::Render(gfx::Canvas* canvas,
                          const TextSelectionModel& selection_model) {
  DCHECK(!ShouldFormat());
  const auto selection = TextFormatter::FormatSelection(buffer_,
                                                        selection_model);
  screen_text_block_->Render(canvas, text_block_.get(), selection);
  RenderRuler(canvas);
  format_counter_ = text_block_->format_counter();
  should_render_ = false;
}

void TextRenderer::RenderRuler(gfx::Canvas* canvas) {
  // FIXME 2007-08-05 We should expose show/hide and ruler settings to both
  // script and UI.
  auto style = buffer_->GetDefaultStyle();
  style.set_font_size(style.font_size() * zoom_);
  auto const font = FontSet::GetFont(style, 'x');

  auto const num_columns = 81;
  auto const width_of_M = font->GetCharWidth('M');
  auto const ruler_x = ::floor(bounds_.left + width_of_M * num_columns);
  auto const ruler_bounds = gfx::RectF (
      gfx::PointF(ruler_x, bounds_.top), gfx::SizeF(1.0f, bounds_.height()));

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, ruler_bounds);
  gfx::Brush brush(canvas, gfx::ColorF(0, 0, 0, 0.3f));
  canvas->DrawRectangle(brush, ruler_bounds);
}

void TextRenderer::RenderSelectionIfNeeded(
    gfx::Canvas* canvas,
    const TextSelectionModel& new_selection_model) {
  DCHECK(!ShouldFormat());
  DCHECK(!should_render_);
  screen_text_block_->RenderSelectionIfNeeded(canvas,
      TextFormatter::FormatSelection(buffer_, new_selection_model));
}

bool TextRenderer::ScrollDown() {
  if (!text_block_->ScrollDown())
    return false;
  should_render_ = true;
  return true;
}

bool TextRenderer::ScrollToPosition(text::Posn offset) {
  return text_block_->ScrollToPosition(offset);
}

bool TextRenderer::ScrollUp() {
  if (!text_block_->ScrollUp())
    return false;
  should_render_ = true;
  return true;
}

void TextRenderer::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_.size() == new_bounds.size())
    return;
  bounds_ = new_bounds;
  text_block_->SetBounds(bounds_);
  screen_text_block_->SetBounds(bounds_);
  should_render_ = true;
}

void TextRenderer::SetZoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  text_block_->SetZoom(zoom_);
}

bool TextRenderer::ShouldFormat() const {
  return text_block_->ShouldFormat();
}

bool TextRenderer::ShouldRender() const {
  return should_render_ || screen_text_block_->dirty();
}
text::Posn TextRenderer::StartOfLine(text::Posn text_offset) const {
  return text_block_->StartOfLine(text_offset);
}

}  // namespace views
