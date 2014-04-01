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
//
//                    |view_rect_|
//  |text_| start ... |text_rect_| ... end
//                     ^
//                     |view_text_start_|
//
class TextFieldControl::Renderer {
  // Text range from 0 to |clean_offset_|, exclusive, of |text_| equals to
  // |text_layout_|.
  private: size_t clean_offset_;
  private: gfx::RectF rect_;
  private: Style style_;
  private: Selection selection_;
  private: base::string16 text_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;
  // Text dimension of all text in one line.
  private: gfx::SizeF text_size_;
  private: gfx::RectF view_rect_;
  // Rectangle of rendered text in |text_layout_| coordinate.
  private: gfx::RectF view_text_rect_;
  // Text offset of first character of rendered text.
  private: size_t view_text_start_;

  public: Renderer(const base::string16& text, const Style& style,
                   const Selection& selection);
  public: ~Renderer();

  public: void set_selection(const Selection& selection);
  public: void set_style(const Style& style);
  public: void set_text(const base::string16& text);
  private: gfx::PointF text_origin() const;

  private: void MakeSelectionVisible();
  public: void Render(gfx::Graphics* gfx, bool has_focus,
                      Control::State state);
  private: void RenderCaret(gfx::Graphics* gfx,
                            const gfx::RectF& caret_rect) const;
  private: void RenderSelection(gfx::Graphics* gfx, bool has_focus);
  private: void ResetTextLayout();
  private: void ResetViewPort();
  public: void ResizeTo(const gfx::RectF& rect);
  private: void UpdateTextLayout();

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const TextFieldControl::Style& style, float height) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  const auto kHugeWidth = 1e6f;
  return text_format.CreateLayout(text, gfx::SizeF(kHugeWidth, height));
}
}  // namespace

TextFieldControl::Renderer::Renderer(const base::string16& text,
                                     const Style& style,
                                     const Selection& selection)
    : selection_(selection), style_(style), text_(text) {
  ResetViewPort();
  ResetTextLayout();
}

TextFieldControl::Renderer::~Renderer() {
}

void TextFieldControl::Renderer::set_selection(
    const Selection& new_selection) {
  if (selection_ == new_selection)
    return;
  selection_ = new_selection;
}

void TextFieldControl::Renderer::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  if (style_.font_size != new_style.font_size)
    ResetTextLayout();
  style_ = new_style;
}

void TextFieldControl::Renderer::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  DCHECK_EQ(base::string16::npos, new_text.find('\n'));
  for (auto offset = 0u; offset < clean_offset_; ++offset) {
    if (offset == text_.size() || offset == new_text.size() ||
        text_[offset] != new_text[offset]) {
      clean_offset_ = offset;
      break;
    }
  }
  text_ = new_text;
  if (view_text_start_ >= clean_offset_)
    ResetViewPort();
  ResetTextLayout();
}

gfx::PointF TextFieldControl::Renderer::text_origin() const {
  return view_rect_.left_top() - view_text_rect_.left_top() +
         gfx::SizeF(0.0f, (view_rect_.height() - text_size_.height) / 2);
}

void TextFieldControl::Renderer::MakeSelectionVisible() {
  DCHECK(view_text_rect_);
  if (text_size_.width <= view_rect_.width()) {
    // We can display whole text in |view_rect_|.
    view_text_start_ = 0;
    view_text_rect_.left = 0.0f;
    view_text_rect_.right = view_text_rect_.width();
    return;
  }

  {
    auto caret_x = 0.0f;
    auto caret_y = 0.0f;
    auto const is_trailing = true;
    DWRITE_HIT_TEST_METRICS metrics = {0};
    COM_VERIFY((*text_layout_)->HitTestTextPosition(
        static_cast<uint32_t>(selection_.focus_offset), is_trailing,
        &caret_x, &caret_y, &metrics));
    if (caret_x <= view_rect_.width()) {
      // Caret is in left most text part.
      view_text_start_ = 0;
      view_text_rect_.left = 0.0f;
      view_text_rect_.right = view_rect_.width();
      return;
    }
    auto const caret = gfx::PointF(caret_x, caret_y);
    if (view_text_rect_.Contains(caret)) {
      // A character after caret is in view port.
      return;
    }

    // Place caret at right edge of |view_rect_|.
    view_text_rect_.left = caret_x - view_rect_.width();
    view_text_rect_.right = caret_x;
  }

  // Compute text offset of view port.
  BOOL is_inside = false;
  BOOL is_trailing = false;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  COM_VERIFY((*text_layout_)->HitTestPoint(
      view_text_rect_.left, 0.0f,
      &is_trailing, &is_inside, &metrics));
  view_text_start_ = metrics.textPosition;
}

void TextFieldControl::Renderer::Render(gfx::Graphics* gfx, bool has_focus,
                                        Control::State state) {
  if (!rect_)
    return;

  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect_);

  // Render frame
  const auto frame_rect = rect_;
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, frame_rect);
  gfx->DrawRectangle(gfx::Brush(*gfx, style_.shadow), frame_rect);

  if (!text_layout_) {
    UpdateTextLayout();
    if (!text_layout_)
      return;
  }

  MakeSelectionVisible();

  // Render text
  {
    gfx::Brush text_brush(*gfx, state == State::Disabled ? style_.gray_text :
                                                           style_.color);
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, view_rect_);
    (*gfx)->DrawTextLayout(text_origin(), *text_layout_, text_brush,
                           D2D1_DRAW_TEXT_OPTIONS_CLIP);
  }

  RenderSelection(gfx, has_focus);

  // Render state
  switch (state) {
    case Control::State::Disabled:
      gfx->FillRectangle(
          gfx::Brush(*gfx, gfx::ColorF(style_.shadow, 0.1f)),
          frame_rect);
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

void TextFieldControl::Renderer::RenderCaret(
    gfx::Graphics* gfx, const gfx::RectF& caret_rect) const {
  // TODO(yosi) We should ask global caret controller for blinking caret.
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, caret_rect);
  gfx::Brush caret_brush(*gfx, gfx::ColorF(style_.color));
  (*gfx)->DrawRectangle(caret_rect, caret_brush);
}

void TextFieldControl::Renderer::RenderSelection(gfx::Graphics* gfx,
                                                 bool has_focus) {
  const auto text_origin = this->text_origin();
  if (selection_.collapsed()) {
    if (!has_focus)
      return;
    auto caret_x = 0.0f;
    auto caret_y = 0.0f;
    auto const is_trailing = false;
    DWRITE_HIT_TEST_METRICS metrics;
    COM_VERIFY((*text_layout_)->HitTestTextPosition(
        static_cast<uint32_t>(selection_.focus_offset), is_trailing,
        &caret_x, &caret_y, &metrics));
    const auto caret = gfx::PointF(caret_x, caret_y) + text_origin;
    RenderCaret(gfx, gfx::RectF(caret, gfx::SizeF(1.0f, metrics.height)));
    return;
  }

  DWRITE_HIT_TEST_METRICS metrics;
  uint32_t num_metrics = 0;
  COM_VERIFY((*text_layout_)->HitTestTextRange(
      static_cast<uint32_t>(selection_.start()),
      static_cast<uint32_t>(selection_.end() - selection_.start()),
      text_origin.x, text_origin.y,
      &metrics, 1u, &num_metrics));
  DCHECK_EQ(1u, num_metrics);
  auto const fill_color = has_focus ? style_.highlight : style_.gray_text;
  const auto range_rect = gfx::RectF(
      gfx::PointF(metrics.left, metrics.top),
      gfx::SizeF(metrics.width, metrics.height));
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF(fill_color, 0.3f)),
                     range_rect);
  if (!has_focus)
    return;
  RenderCaret(gfx, gfx::RectF(
      selection_.focus_offset < selection_.anchor_offset ?
          range_rect.left_top() :
          gfx::PointF(range_rect.right, range_rect.top),
      gfx::SizeF(1.0f, metrics.height)));
}

void TextFieldControl::Renderer::ResetTextLayout() {
  text_layout_.reset();
  clean_offset_ = 0;
}

void TextFieldControl::Renderer::ResetViewPort() {
  view_text_start_ = 0;
  view_text_rect_ = gfx::RectF();
}

void TextFieldControl::Renderer::ResizeTo(const gfx::RectF& new_rect) {
  if (rect_ == new_rect)
    return;
  ResetTextLayout();
  rect_ = new_rect;
  view_rect_ = gfx::RectF(new_rect.left + padding_left,
                          new_rect.top + padding_top,
                          new_rect.right - padding_right,
                          new_rect.bottom - padding_bottom);
}

void TextFieldControl::Renderer::UpdateTextLayout() {
  DCHECK(!text_layout_);
  text_layout_ = CreateTextLayout(text_, style_, view_rect_.height());
  if (!text_layout_)
    return;
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  DCHECK_EQ(1u, metrics.lineCount);
  clean_offset_ = text_.size();
  text_size_ = gfx::SizeF(metrics.width, metrics.height);
  view_text_rect_.top = 0.0f;
  view_text_rect_.bottom = text_size_.height;
  view_text_rect_.right = view_text_rect_.left + view_rect_.width();
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl::Selection
//
TextFieldControl::Selection::Selection() : anchor_offset(0), focus_offset(0) {
}

TextFieldControl::Selection::~Selection() {
}

bool TextFieldControl::Selection::operator==(const Selection& other) const {
  return anchor_offset == other.anchor_offset &&
         focus_offset == other.focus_offset;
}

bool TextFieldControl::Selection::operator!=(const Selection& other) const {
  return !operator==(other);
}

bool TextFieldControl::Selection::collapsed() const {
  return anchor_offset == focus_offset;
}

size_t TextFieldControl::Selection::end() const {
  return std::max(anchor_offset, focus_offset);
}

size_t TextFieldControl::Selection::start() const {
  return std::min(anchor_offset, focus_offset);
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
TextFieldControl::TextFieldControl(ControlController* controller,
                                   const Selection& selection,
                                   const base::string16& text,
                                   const Style& style)
    : Control(controller), renderer_(new Renderer(text, style, selection)) {
}

TextFieldControl::~TextFieldControl() {
}

void TextFieldControl::set_selection(const Selection& new_selection) {
  renderer_->set_selection(new_selection);
}

void TextFieldControl::set_style(const Style& new_style) {
  renderer_->set_style(new_style);
}

void TextFieldControl::set_text(const base::string16& new_text) {
  renderer_->set_text(new_text);
}

// ui::Widget
void TextFieldControl::DidResize() {
  renderer_->ResizeTo(gfx::RectF(rect()));
}

void TextFieldControl::OnDraw(gfx::Graphics* gfx) {
  renderer_->Render(gfx, has_focus(), state());
}

}  // namespace ui
