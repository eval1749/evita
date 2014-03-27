// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/text_field_control.h"

#include "evita/gfx_base.h"

namespace ui {

namespace {
auto const padding_bottom = 1.0f;
auto const padding_left = 5.0f;
auto const padding_right = 5.0f;
auto const padding_top = 1.0f;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl::Renderer
//
class TextFieldControl::Renderer {
  private: gfx::RectF rect_;
  private: Style style_;
  private: gfx::RectF layout_rect_;
  private: gfx::SizeF text_size_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Renderer(const base::string16& text, const Style& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Graphics* gfx, bool has_focus,
                      const Selection& selection,
                      Control::State state) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const TextFieldControl::Style& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  return text_format.CreateLayout(text, size);
}
}  // namespace

TextFieldControl::Renderer::Renderer(const base::string16& text,
                                     const Style& style,
                                     const gfx::RectF& rect)
    : rect_(rect), style_(style),
      layout_rect_(rect.left + padding_left,
                 rect.top + padding_top,
                 rect.right - padding_right,
                 rect.bottom - padding_bottom),
      text_layout_(CreateTextLayout(text, style, layout_rect_.size())) {
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  // Event after trimming of |text_|, it may not fit into line. Although,
  // DCHECK_EQ(1u, metrics.lineCount);
  text_size_ = gfx::SizeF(metrics.width, metrics.height);
}

TextFieldControl::Renderer::~Renderer() {
}

void TextFieldControl::Renderer::Render(gfx::Graphics* gfx, bool has_focus,
                                        const Selection& selection,
                                        Control::State state) const {
  if (!rect_)
    return;

  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect_);

  // Render frame
  const auto frame_rect = rect_;
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, frame_rect);
  gfx->DrawRectangle(gfx::Brush(*gfx, style_.shadow), frame_rect);

  // Render text
    auto const offset = (frame_rect.size() - text_size_) / 2.0f;
    gfx::PointF text_origin(layout_rect_.left,
                            layout_rect_.top + offset.height);
  {
    gfx::Brush text_brush(*gfx, state == State::Disabled ? style_.gray_text :
                                                           style_.color);
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, layout_rect_);
    (*gfx)->DrawTextLayout(text_origin, *text_layout_, text_brush,
                           D2D1_DRAW_TEXT_OPTIONS_CLIP);
  }

  // Render selection
  if (selection.start != selection.end) {
    DWRITE_HIT_TEST_METRICS metrics;
    uint32_t num_metrics = 0;
    COM_VERIFY((*text_layout_)->HitTestTextRange(
        static_cast<uint32_t>(selection.start),
        static_cast<uint32_t>(selection.end - selection.start),
        text_origin.x, text_origin.y,
        &metrics, 1u, &num_metrics));
    if (num_metrics == 1) {
      auto const fill_color = has_focus ? style_.highlight : style_.gray_text;
      gfx->FillRectangle(
        gfx::Brush(*gfx, gfx::ColorF(fill_color, 0.3f)),
        gfx::RectF(gfx::PointF(metrics.left, metrics.top),
                   gfx::SizeF(metrics.width, metrics.height)));
    }
  }

  // Render caret
  if (has_focus) {
    auto caret_x = 0.0f;
    auto caret_y = 0.0f;
    DWRITE_HIT_TEST_METRICS metrics;
    auto use_start = selection.start == selection.end ||
                     selection.start_is_active;
    COM_VERIFY((*text_layout_)->HitTestTextPosition(
        static_cast<uint32_t>(use_start ? selection.start : selection.end - 1),
        !use_start, &caret_x, &caret_y, &metrics));
    const auto caret_left_top = gfx::PointF(caret_x, caret_y) + text_origin;
    (*gfx)->DrawLine(caret_left_top,
                     caret_left_top + gfx::SizeF(0.0f, metrics.height),
                     gfx::Brush(*gfx, gfx::ColorF(style_.color)));
  }

  // Render state
  switch (state) {
    case Control::State::Disabled:
      gfx->FillRectangle(
          gfx::Brush(*gfx, gfx::ColorF(style_.gray_text, 0.5f)),
          frame_rect);
      gfx->DrawRectangle(gfx::Brush(*gfx, style_.color), frame_rect);
      break;
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      gfx->DrawRectangle(gfx::Brush(*gfx, gfx::ColorF(style_.highlight, 0.5f)),
                         frame_rect - 1.0f, 2.0f);
      break;
    case Control::State::Hover:
      gfx->DrawRectangle(gfx::Brush(*gfx, style_.hotlight), frame_rect);
      break;
  }
  gfx->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl::Selection
//
bool TextFieldControl::Selection::operator==(const Selection& other) const {
  return end == other.end && start == other.start &&
         start_is_active == other.start_is_active;
}

bool TextFieldControl::Selection::operator!=(const Selection& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
TextFieldControl::TextFieldControl(ControlController* controller,
                                   const Selection& selection,
                                   const base::string16& text,
                                   const Style& style)
    : Control(controller), selection_(selection), style_(style), text_(text) {
}

TextFieldControl::~TextFieldControl() {
}

void TextFieldControl::set_selection(const Selection& new_selection) {
  if (selection_ == new_selection)
    return;
  selection_ = new_selection;
}

void TextFieldControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  renderer_.reset();
}

void TextFieldControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  renderer_.reset();
}

// ui::Widget
void TextFieldControl::DidResize() {
  renderer_.reset();
}

void TextFieldControl::OnDraw(gfx::Graphics* gfx) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, gfx::RectF(rect()));
  renderer_->Render(gfx, has_focus(), selection_, state());
}

}  // namespace ui
