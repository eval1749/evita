// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/views/message_view.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"

#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/animation/animation_value.h"
#include "evita/ui/controls/status_bar.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/system_metrics.h"

namespace views {

namespace {

auto const kMinMainPartWidth = 200.0f;

struct PartView {
  float alpha;
  base::string16 text;

  PartView(float alpha, const base::string16& text);
  PartView();
  ~PartView() = default;
};

PartView::PartView(float alpha, const base::string16& text)
    : alpha(alpha), text(text) {}

PartView::PartView() : PartView(1.0f, base::string16()) {}

// Paint resize button with six dots:
//  --o
//  -oo
//  ooo
void PaintResizeButton(gfx::Canvas* canvas,
                       const gfx::RectF& bounds,
                       float alpha) {
  auto const bounds2 = bounds - gfx::SizeF(2.0f, 2.0f);
  auto const dot_size = 2.0f;
  for (auto y = 1; y <= 3; ++y) {
    for (auto x = 4 - y; x >= 1; --x) {
      auto const dot =
          gfx::RectF(gfx::PointF(bounds2.right - x * (dot_size + 1),
                                 bounds2.bottom - y * (dot_size + 1)),
                     gfx::SizeF(dot_size, dot_size));
      gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, dot);
      canvas->Clear(gfx::sysColor(COLOR_SCROLLBAR, alpha));
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// Painter
//
class Painter final {
 public:
  Painter();
  ~Painter() = default;

  void Paint(gfx::Canvas* canvas, const std::vector<PartView>& parts);

 private:
  struct Part {
    float alpha;
    gfx::RectF bounds;
    bool dirty;
    PartView model;
    base::string16 text;
    gfx::TextLayout* text_layout;
    float width;

    Part();
    ~Part();
  };

  gfx::RectF bounds_;
  float original_main_part_width_;
  std::vector<Part> parts_;
  std::unique_ptr<gfx::TextFormat> text_format_;

  DISALLOW_COPY_AND_ASSIGN(Painter);
};

auto const kPaddingRight = 8.0f;

Painter::Part::Part()
    : alpha(1), dirty(false), text_layout(nullptr), width(kPaddingRight) {}

Painter::Part::~Part() {
  delete text_layout;
}

Painter::Painter() : original_main_part_width_(0) {
  text_format_.reset(
      new gfx::TextFormat(ui::SystemMetrics::instance()->font_family(),
                          ui::SystemMetrics::instance()->font_size()));
  (*text_format_)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void Painter::Paint(gfx::Canvas* canvas,
                    const std::vector<PartView>& new_parts) {
  auto const new_bounds = canvas->GetLocalBounds();
  auto dirty = bounds_ != new_bounds;
  if (parts_.size() != new_parts.size()) {
    parts_.clear();
    parts_.resize(new_parts.size());
  }

  auto const button_size = ::GetSystemMetrics(SM_CXVSCROLL);
  auto const new_resize_button_bounds = gfx::RectF(
      new_bounds.bottom_right() - gfx::SizeF(button_size, button_size),
      new_bounds.bottom_right());
  auto const new_parts_bounds =
      gfx::RectF(new_bounds.origin() + gfx::SizeF(0, 1),
                 gfx::PointF(new_resize_button_bounds.left, new_bounds.bottom));

  // Update part width
  {
    auto part = parts_.begin();
    auto const big_size = gfx::SizeF(10000.0f, new_parts_bounds.height());
    auto total_width = 0.0f;
    auto const main_part_width = parts_[0].width;
    parts_[0].width = original_main_part_width_;
    for (const auto& new_part : new_parts) {
      if (part->text != new_part.text) {
        part->dirty = true;
        part->text = new_part.text;
        delete part->text_layout;
        part->text_layout = nullptr;
        if (part->text.empty()) {
          part->width = 0.0f;
        } else {
          auto text_layout = text_format_->CreateLayout(part->text, big_size);
          part->text_layout = text_layout.release();
          DWRITE_TEXT_METRICS metrics;
          COM_VERIFY((*part->text_layout)->GetMetrics(&metrics));
          part->width =
              std::max(::ceil(metrics.width) + kPaddingRight, part->width);
        }
        dirty = true;
      }
      total_width += part->width;
      if (part->alpha != new_part.alpha) {
        part->alpha = new_part.alpha;
        part->dirty = true;
        dirty = true;
      }
      ++part;
    }
    auto const new_main_part_width =
        std::max(new_parts_bounds.width() - total_width + parts_[0].width,
                 std::min(std::max(parts_[0].width, kMinMainPartWidth),
                          new_parts_bounds.width()));
    original_main_part_width_ = parts_[0].width;
    parts_[0].width = main_part_width;
    if (parts_[0].width != new_main_part_width) {
      parts_[0].width = new_main_part_width;
      parts_[0].dirty = true;
      dirty = true;
    }
  }

  // Update part bounds
  {
    auto origin = new_parts_bounds.origin();
    for (auto& part : parts_) {
      // Part bounds don't contain top border line.
      auto const new_part_size =
          gfx::SizeF(part.width, new_parts_bounds.height());
      auto const new_part_bounds =
          new_parts_bounds.Intersect(gfx::RectF(origin, new_part_size));
      if (part.bounds != new_part_bounds) {
        part.bounds = new_part_bounds;
        part.dirty = true;
        dirty = true;
      }
      origin = part.bounds.top_right();
    }
    DCHECK_GE(origin.x, new_parts_bounds.right);
  }

  if (!dirty)
    return;

  auto const alpha = 0.8f;
  if (bounds_ != new_bounds) {
    bounds_ = new_bounds;
    canvas->Clear(gfx::sysColor(COLOR_BTNFACE, alpha));
    canvas->DrawLine(gfx::Brush(canvas, gfx::sysColor(COLOR_BTNSHADOW, alpha)),
                     bounds_.origin(), bounds_.top_right());
    PaintResizeButton(canvas, new_resize_button_bounds, alpha);
    canvas->AddDirtyRect(bounds_);
    for (auto& part : parts_) {
      part.dirty = true;
    }
  }

  // Paint parts
  for (auto& part : parts_) {
    if (!part.dirty)
      continue;
    part.dirty = false;
    if (part.bounds.empty())
      continue;
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, part.bounds);
    canvas->AddDirtyRect(part.bounds);
    canvas->Clear(gfx::sysColor(COLOR_BTNFACE, alpha));
    if (!part.alpha || !part.text_layout)
      continue;
    gfx::Brush text_brush(canvas, gfx::sysColor(COLOR_BTNTEXT, part.alpha));
    (*canvas)->DrawTextLayout(part.bounds.origin() + gfx::SizeF(4, 0),
                              *part.text_layout, text_brush);
    canvas->Flush();
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MessageView::View
//
class MessageView::View final {
 public:
  explicit View(ui::AnimatableWindow* animator);
  ~View() = default;

  void Animate(base::Time time);
  void SetMessage(const base::string16& text);
  void SetStatus(const std::vector<base::string16>& texts);

 private:
  ui::AnimatableWindow* const animator_;
  std::unique_ptr<gfx::Canvas> canvas_;
  std::unique_ptr<Painter> painter_;
  std::vector<PartView> parts_;
  std::unique_ptr<ui::AnimationFloat> main_text_alpha_;
  base::string16 message_text_;
  base::string16 status_text_;

  DISALLOW_COPY_AND_ASSIGN(View);
};

MessageView::View::View(ui::AnimatableWindow* animator)
    : animator_(animator), painter_(new Painter()), parts_(1) {}

void MessageView::View::Animate(base::Time now) {
  if (!animator_->visible())
    return;

  if (!canvas_)
    canvas_.reset(animator_->layer()->CreateCanvas());
  else if (canvas_->GetLocalBounds() != animator_->GetContentsBounds())
    canvas_->SetBounds(animator_->GetContentsBounds());

  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  if (canvas_->should_clear())
    canvas_->Clear(gfx::ColorF());

  if (!main_text_alpha_) {
    // Hide main text after 5 seconds.
    main_text_alpha_.reset(new ui::AnimationFloat(
        now, base::TimeDelta::FromSeconds(5), 1.0f, 0.0f));
    parts_[0].text = message_text_.empty() ? status_text_ : message_text_;
  }

  auto const new_alpha = main_text_alpha_->Compute(now);
  parts_[0].alpha = new_alpha;
  painter_->Paint(canvas_.get(), parts_);
  if (new_alpha != main_text_alpha_->end_value()) {
    animator_->RequestAnimationFrame();
    return;
  }

  // Main text animation is finished.
  if (message_text_.empty())
    return;
  // We'll display status text
  main_text_alpha_.reset();
  message_text_.clear();
  animator_->RequestAnimationFrame();
}

void MessageView::View::SetMessage(const base::string16& text) {
  if (message_text_ == text)
    return;
  message_text_ = text;
  main_text_alpha_.reset();
  animator_->RequestAnimationFrame();
}

void MessageView::View::SetStatus(const std::vector<base::string16>& texts) {
  DCHECK(!texts.empty());
  status_text_ = texts[0];
  const auto current_text = parts_[0].text;
  parts_.clear();
  for (auto const text : texts) {
    PartView part_model{1.0f, text};
    parts_.push_back(part_model);
  }
  parts_[0].text = current_text;
  main_text_alpha_.reset();
  animator_->RequestAnimationFrame();
}

//////////////////////////////////////////////////////////////////////
//
// MessageView
//
MessageView::MessageView() : view_(new View(this)) {}

MessageView::~MessageView() {}

void MessageView::SetMessage(const base::string16& text) {
  view_->SetMessage(text);
}

void MessageView::SetStatus(const std::vector<base::string16>& texts) {
  view_->SetStatus(texts);
}

// ui::AnimationFrameHandler
void MessageView::DidBeginAnimationFrame(base::Time time) {
  TRACE_EVENT0("view", "MessageView::DidBeginAnimationFrame");
  view_->Animate(time);
}

// ui::Widget
void MessageView::DidRealize() {
  ui::AnimatableWindow::DidRealize();
  SetLayer(new ui::Layer());
}

gfx::Size MessageView::GetPreferredSize() const {
  return gfx::Size(0, ::GetSystemMetrics(SM_CYCAPTION));
}

}  // namespace views
