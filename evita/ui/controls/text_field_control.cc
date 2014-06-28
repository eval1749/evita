// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/text_field_control.h"

#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/caret.h"

namespace ui {

namespace {
auto const padding_bottom = 1.0f;
auto const padding_left = 5.0f;
auto const padding_right = 5.0f;
auto const padding_top = 1.0f;

inline gfx::RectF RoundBounds(const gfx::RectF& bounds) {
  return gfx::RectF(::floor(bounds.left), ::floor(bounds.top),
                    ::floor(bounds.right), ::floor(bounds.bottom));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl::Renderer
//
//
//                    |view_bounds_|
//  |text_| start ... |text_bounds_| ... end
//                     ^
//                     |view_text_start_|
//
class TextFieldControl::Renderer final : private Caret::Delegate {
  private: gfx::RectF bounds_;
  // Text range from 0 to |clean_offset_|, exclusive, of |text_| equals to
  // |text_layout_|.
  private: size_t clean_offset_;
  private: Style style_;
  private: Selection selection_;
  private: Control::State state_;
  private: base::string16 text_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;
  // Text dimension of all text in one line.
  private: gfx::SizeF text_size_;
  private: gfx::RectF view_bounds_;
  // Rectangle of rendered text in |text_layout_| coordinate.
  private: gfx::RectF view_text_bounds_;
  // Text offset of first character of rendered text.
  private: size_t view_text_start_;

  public: Renderer(const base::string16& text, const Style& style,
                   const Selection& selection);
  public: ~Renderer();

  public: void set_selection(const Selection& selection);
  public: void set_state(Control::State new_state) { state_ = new_state; }
  public: void set_style(const Style& style);
  public: void set_text(const base::string16& text);
  private: gfx::PointF text_origin() const;

  public: void DidKillFocus();
  public: void DidSetFocus();
  private: void MakeSelectionVisible();
  public: int Renderer::MapPointToOffset(const gfx::PointF& point) const;
  public: void Render(gfx::Canvas* canvas);
  private: void RenderCaret(gfx::Canvas* gfx, const gfx::RectF& caret_rect);
  private: void RenderSelection(gfx::Canvas* gfx);
  private: void ResetTextLayout();
  private: void ResetViewPort();
  public: void SetBounds(const gfx::RectF& rect);
  private: void UpdateTextLayout();

  // ui::Caret::Delegate
  private: virtual void HideCaret(gfx::Canvas* canvas,
                                  const Caret& caret) override;
  private: virtual void PaintCaret(gfx::Canvas* canvas,
                                   const Caret& caret) override;
  private: virtual void ShowCaret(gfx::Canvas* canvas,
                                  const Caret& caret) override;

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
    : selection_(selection), state_(Control::State::Normal), style_(style),
      text_(text) {
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
  return view_bounds_.left_top() - view_text_bounds_.left_top() +
         gfx::SizeF(0.0f, (view_bounds_.height() - text_size_.height) / 2);
}

void TextFieldControl::Renderer::DidKillFocus() {
  Caret::instance()->Give(this);
}

void TextFieldControl::Renderer::DidSetFocus() {
  Caret::instance()->Take(this);
}

void TextFieldControl::Renderer::MakeSelectionVisible() {
  DCHECK(view_text_bounds_);
  if (text_size_.width <= view_bounds_.width()) {
    // We can display whole text in |view_bounds_|.
    view_text_start_ = 0;
    view_text_bounds_.left = 0.0f;
    view_text_bounds_.right = view_text_bounds_.width();
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
    if (caret_x <= view_bounds_.width()) {
      // Caret is in left most text part.
      view_text_start_ = 0;
      view_text_bounds_.left = 0.0f;
      view_text_bounds_.right = view_bounds_.width();
      return;
    }
    auto const caret = gfx::PointF(caret_x, caret_y);
    if (view_text_bounds_.Contains(caret)) {
      // A character after caret is in view port.
      return;
    }

    // Place caret at right edge of |view_bounds_|.
    view_text_bounds_.left = caret_x - view_bounds_.width();
    view_text_bounds_.right = caret_x;
  }

  // Compute text offset of view port.
  BOOL is_inside = false;
  BOOL is_trailing = false;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  COM_VERIFY((*text_layout_)->HitTestPoint(
      view_text_bounds_.left, 0.0f,
      &is_trailing, &is_inside, &metrics));
  view_text_start_ = metrics.textPosition;
}

int TextFieldControl::Renderer::MapPointToOffset(
    const gfx::PointF& window_point) const {
  BOOL is_inside = false;
  BOOL is_trailing = false;
  DWRITE_HIT_TEST_METRICS metrics = {0};
  auto const origin = text_origin();
  auto point = window_point - origin;
  COM_VERIFY((*text_layout_)->HitTestPoint(
      point.x, point.y, &is_trailing, &is_inside, &metrics));
  return static_cast<int>(metrics.textPosition + is_trailing);
}

void TextFieldControl::Renderer::Render(gfx::Canvas* gfx) {
  if (bounds_.empty())
    return;

  Caret::instance()->DidPaint(this, bounds_);
  gfx->set_dirty_rect(bounds_);
  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), bounds_);

  // Render frame
  const auto frame_rect = bounds_;
  {
    gfx::Canvas::AxisAlignedClipScope clip_scope(*gfx, frame_rect);
    gfx->DrawRectangle(gfx::Brush(*gfx, style_.shadow), frame_rect);
  }

  if (!text_layout_) {
    UpdateTextLayout();
    if (!text_layout_)
      return;
  }

  MakeSelectionVisible();

  // Render text
  {
    gfx::Brush text_brush(*gfx, state_ == State::Disabled ? style_.gray_text :
                                                            style_.color);
    gfx::Canvas::AxisAlignedClipScope clip_scope(*gfx, view_bounds_);
    (*gfx)->DrawTextLayout(text_origin(), *text_layout_, text_brush,
                           D2D1_DRAW_TEXT_OPTIONS_CLIP);
  }

  if (Caret::instance()->owner() == this)
    RenderSelection(gfx);

  // Render state
  gfx::Canvas::AxisAlignedClipScope clip_scope(*gfx, bounds_);
  switch (state_) {
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
    gfx::Canvas* canvas, const gfx::RectF& caret_bounds) {
  const auto bounds = RoundBounds(caret_bounds);
  Caret::instance()->Update(this, canvas, bounds);
}

void TextFieldControl::Renderer::RenderSelection(gfx::Canvas* gfx) {
  const auto text_origin = this->text_origin();
  if (selection_.collapsed()) {
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
      0.0f, 0.0f,
      &metrics, 1u, &num_metrics));
  DCHECK_EQ(1u, num_metrics);
  auto const fill_color = style_.highlight;
  const auto range_rect = gfx::RectF(
      std::max(metrics.left, view_text_bounds_.left), metrics.top,
      std::min(metrics.left + metrics.width, view_text_bounds_.right),
      metrics.top + metrics.height) + text_origin;
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF(fill_color, 0.3f)),
                     range_rect);
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
  view_text_bounds_ = gfx::RectF();
}

void TextFieldControl::Renderer::SetBounds(const gfx::RectF& new_rect) {
  if (bounds_ == new_rect)
    return;
  ResetTextLayout();
  bounds_ = new_rect;
  view_bounds_ = gfx::RectF(new_rect.left + padding_left,
                          new_rect.top + padding_top,
                          new_rect.right - padding_right,
                          new_rect.bottom - padding_bottom);
}

void TextFieldControl::Renderer::UpdateTextLayout() {
  DCHECK(!text_layout_);
  text_layout_ = CreateTextLayout(text_, style_, view_bounds_.height());
  if (!text_layout_)
    return;
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  DCHECK_EQ(1u, metrics.lineCount);
  clean_offset_ = text_.size();
  text_size_ = gfx::SizeF(metrics.width, metrics.height);
  view_text_bounds_.top = 0.0f;
  view_text_bounds_.bottom = text_size_.height;
  view_text_bounds_.right = view_text_bounds_.left + view_bounds_.width();
}

// ui::Caret::Delegate
void TextFieldControl::Renderer::HideCaret(gfx::Canvas* canvas, const Caret&) {
  gfx::Canvas::DrawingScope drawing_scope(*canvas);
  Render(canvas);
}

void TextFieldControl::Renderer::PaintCaret(gfx::Canvas* canvas,
                                            const Caret& caret) {
  gfx::Brush fill_brush(*canvas, gfx::ColorF::Black);
  canvas->FillRectangle(fill_brush, caret.bounds());
}

void TextFieldControl::Renderer::ShowCaret(gfx::Canvas* canvas,
                                           const Caret& caret) {
  gfx::Canvas::DrawingScope drawing_scope(*canvas);
  canvas->set_dirty_rect(caret.bounds());
  PaintCaret(canvas, caret);
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
    : Control(controller),
      renderer_(new Renderer(text, style, selection)) {
}

TextFieldControl::~TextFieldControl() {
}

void TextFieldControl::set_selection(const Selection& new_selection) {
  renderer_->set_selection(new_selection);
  SchedulePaint();
}

void TextFieldControl::set_style(const Style& new_style) {
  renderer_->set_style(new_style);
  SchedulePaint();
}

void TextFieldControl::set_text(const base::string16& new_text) {
  renderer_->set_text(new_text);
  SchedulePaint();
}

int TextFieldControl::MapPointToOffset(const gfx::PointF& point) const {
  return renderer_->MapPointToOffset(point);
}

// ui::Control
void TextFieldControl::DidChangeState() {
  renderer_->set_state(state());
  Control::DidChangeState();
}

// ui::Widget
void TextFieldControl::DidKillFocus(ui::Widget* focused_widget) {
  renderer_->DidKillFocus();
  SchedulePaint();
  Control::DidKillFocus(focused_widget);
}

void TextFieldControl::DidResize() {
  renderer_->SetBounds(gfx::RectF(bounds()));
}

void TextFieldControl::DidSetFocus(ui::Widget* last_focused_widget) {
  renderer_->DidSetFocus();
  SchedulePaint();
  Control::DidSetFocus(last_focused_widget);
}

HCURSOR TextFieldControl::GetCursorAt(const Point&) const {
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

void TextFieldControl::OnDraw(gfx::Canvas* canvas) {
  renderer_->Render(canvas);
}

}  // namespace ui
