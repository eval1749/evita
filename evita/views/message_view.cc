// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/message_view.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"
#include "evita/ui/controls/status_bar.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/system_metrics.h"

namespace views {

namespace {

auto const kMinMainPartWidth = 200.0f;

struct PartModel {
  float alpha;
  base::string16 text;

  PartModel(float alpha, const base::string16& text);
  PartModel();
  ~PartModel() = default;
};

PartModel::PartModel(float alpha, const base::string16& text)
    : alpha(alpha), text(text) {
}

PartModel::PartModel()
    : PartModel(1.0f, base::string16()) {
}

// Paint resize button with six dots:
//  --o
//  -oo
//  ooo
void PaintResizeButton(gfx::Canvas* canvas, const gfx::RectF& bounds,
                       float alpha) {
  auto const bounds2 = bounds - gfx::SizeF(2.0f, 2.0f);
  auto const dot_size = 2.0f;
  for (auto y = 1; y <= 3; ++y) {
    for (auto x = 4 - y; x >= 1; --x) {
        auto const dot = gfx::RectF(
            gfx::PointF(bounds2.right - x * (dot_size + 1),
                        bounds2.bottom - y * (dot_size + 1)),
            gfx::SizeF(dot_size, dot_size));
        gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, dot);
        canvas->Clear(gfx::sysColor(COLOR_SCROLLBAR, alpha));
     }
  }
  canvas->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// ModelView
//
class ModelView final {
  private: struct Part {
    float alpha;
    gfx::RectF bounds;
    bool dirty;
    PartModel model;
    base::string16 text;
    gfx::TextLayout* text_layout;
    float width;

    Part();
    ~Part();
  };

  private: gfx::RectF bounds_;
  private: float original_main_part_width_;
  private: std::vector<Part> parts_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: ModelView();
  public: ~ModelView() = default;

  public: void Paint(gfx::Canvas* canvas,
                     const gfx::RectF& bounds,
                     const std::vector<PartModel>& parts);

  DISALLOW_COPY_AND_ASSIGN(ModelView);
};

auto const kPaddingRight = 8.0f;

ModelView::Part::Part()
    : alpha(1), dirty(false), text_layout(nullptr), width(kPaddingRight) {
}

ModelView::Part::~Part() {
  delete text_layout;
}

ModelView::ModelView() : original_main_part_width_(0) {
  text_format_.reset(new gfx::TextFormat(
      ui::SystemMetrics::instance()->font_family(),
      ui::SystemMetrics::instance()->font_size()));
  (*text_format_)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void ModelView::Paint(gfx::Canvas* canvas, const gfx::RectF& new_bounds,
                      const std::vector<PartModel>& new_parts) {
  auto dirty = bounds_ != new_bounds;
  if (parts_.size() != new_parts.size()) {
    parts_.clear();
    parts_.resize(new_parts.size());
  }

  auto const button_size = ::GetSystemMetrics(SM_CXVSCROLL);
  auto const new_resize_button_bounds = gfx::RectF(
      new_bounds.bottom_right() - gfx::SizeF(button_size, button_size),
      new_bounds.bottom_right());
  auto const new_parts_bounds = gfx::RectF(
      new_bounds.origin() + gfx::SizeF(0, 1),
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
        auto text_layout = text_format_->CreateLayout(part->text, big_size);
        part->text_layout = text_layout.release();
        DWRITE_TEXT_METRICS metrics;
        COM_VERIFY((*part->text_layout)->GetMetrics(&metrics));
        part->width = std::max(::ceil(metrics.width) + kPaddingRight,
                               part->width);
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
    auto const new_main_part_width = std::max(
        new_parts_bounds.width() - total_width + parts_[0].width,
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
      auto const new_part_size = gfx::SizeF(part.width,
                                            new_parts_bounds.height());
      auto const new_part_bounds = new_parts_bounds.Intersect(
          gfx::RectF(origin, new_part_size));
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

  gfx::Canvas::DrawingScope drawing_scope(canvas);
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
    canvas->AddDirtyRect(part.bounds);
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, part.bounds);
    canvas->Clear(gfx::sysColor(COLOR_BTNFACE, alpha));
    if (!part.alpha || !part.text_layout)
      continue;
    gfx::Brush text_brush(canvas, gfx::sysColor(COLOR_BTNTEXT, part.alpha));
    (*canvas)->DrawTextLayout(part.bounds.origin() + gfx::SizeF(4, 0),
                              *part.text_layout, text_brush);
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MessageView::Model
//
class MessageView::Model final {
  private: std::unique_ptr<ModelView> paint_model_;
  private: std::vector<PartModel> parts_;
  private: base::Time main_text_time_;
  private: base::string16 message_text_;
  private: base::string16 status_text_;

  public: Model();
  public: ~Model() = default;

  private: void Animate(base::Time now);
  public: void SetMessage(const base::string16& text);
  public: void SetStatus(const std::vector<base::string16>& texts);
  private: void StartAnimate(base::Time now, const base::string16& text);
  private: void UpdateLayout();
  public: void UpdateView(gfx::Canvas* canvas, const gfx::RectF& bounds);

  DISALLOW_COPY_AND_ASSIGN(Model);
};

MessageView::Model::Model() : paint_model_(new ModelView()), parts_(1) {
}

void MessageView::Model::Animate(base::Time now) {
  // Hide text after 5000ms.
  auto const kAnimationDuration = 5000.0f;
  auto const duration = now - main_text_time_;
  auto const factor = static_cast<float>(duration.InMilliseconds()) /
                      kAnimationDuration;
  parts_[0].alpha = factor >= 1.0 ? 0.0f : 1.0f - factor;
}

void MessageView::Model::SetMessage(const base::string16& text) {
  message_text_ = text;
}

void MessageView::Model::SetStatus(const std::vector<base::string16>& texts) {
  DCHECK(!texts.empty());
  status_text_ = texts[0];
  const auto current_text = parts_[0].text;
  parts_.clear();
  for (auto const text : texts){
    PartModel part_model {1.0f, text};
    parts_.push_back(part_model);
  }
  parts_[0].text = current_text;
}

void MessageView::Model::StartAnimate(base::Time now,
                                      const base::string16& main_text) {
  // TODO(eval1749) We should request animation frame for hiding main text
  // after 5 second.
  parts_[0].text = main_text;
  parts_[0].alpha = 1.0f;
  main_text_time_ = now;
}

void MessageView::Model::UpdateView(gfx::Canvas* canvas,
                                    const gfx::RectF& bounds) {
  auto const now = base::Time::Now();
  if (!message_text_.empty()) {
    if (message_text_ == parts_[0].text) {
      if (parts_[0].alpha) {
        Animate(now);
      } else {
        message_text_.clear();
        StartAnimate(now, status_text_);
      }
    } else {
      StartAnimate(now, message_text_);
    }
  } else if (status_text_ == parts_[0].text) {
    Animate(now);
  } else {
    StartAnimate(now, status_text_);
  }
  paint_model_->Paint(canvas, bounds, parts_);
}

//////////////////////////////////////////////////////////////////////
//
// MessageView
//
MessageView::MessageView() : model_(new Model()) {
}

MessageView::~MessageView() {
}

void MessageView::SetMessage(const base::string16& text) {
  model_->SetMessage(text);
}

void MessageView::SetStatus(const std::vector<base::string16>& texts) {
  model_->SetStatus(texts);
}

void MessageView::UpdateView() {
  model_->UpdateView(canvas_.get(), GetContentsBounds());
}

// ui::Widget
void MessageView::DidChangeBounds() {
  ui::Widget::DidChangeBounds();
  if (canvas_)
    canvas_->SetBounds(GetContentsBounds());
}

void MessageView::DidRealize() {
  SetLayer(new ui::Layer());
  layer()->SetBounds(gfx::RectF(bounds()));
  canvas_.reset(layer()->CreateCanvas());
}

gfx::Size MessageView::GetPreferredSize() const {
  return gfx::Size(0, ::GetSystemMetrics(SM_CYCAPTION));
}

}  // namespace views
