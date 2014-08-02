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

struct PartModel {
  base::string16 text;
};

//////////////////////////////////////////////////////////////////////
//
// ModelView
//
class ModelView final {
  private: struct Part {
    gfx::RectF bounds;
    bool dirty;
    PartModel model;
    base::string16 text;
    gfx::TextLayout* text_layout;
    float width;

    Part() : dirty(false), text_layout(nullptr), width(1.0f) {
    }

    ~Part() {
      delete text_layout;
    }
  };

  private: gfx::RectF bounds_;
  private: std::vector<Part> parts_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: ModelView();
  public: ~ModelView() = default;

  public: void Paint(gfx::Canvas* canvas,
                     const gfx::RectF& bounds,
                     const std::vector<PartModel>& parts);

  DISALLOW_COPY_AND_ASSIGN(ModelView);
};

ModelView::ModelView() {
  LOGFONT logfont;
  ::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(logfont), &logfont, 0);
  text_format_.reset(new gfx::TextFormat(
      ui::SystemMetrics::instance()->font_family(),
      ui::SystemMetrics::instance()->font_size()));
  (*text_format_)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void ModelView::Paint(gfx::Canvas* canvas, const gfx::RectF& new_bounds,
                      const std::vector<PartModel>& new_parts) {
  auto dirty = bounds_ != new_bounds;
  if (parts_.size() != new_parts.size())
    parts_.resize(new_parts.size());

  // Update part width
  {
    auto part = parts_.begin();
    auto const big_size = gfx::SizeF(10000.0f, new_bounds.height());
    auto total_width = 0.0f;
    for (const auto& new_part : new_parts) {
      if (part->text != new_part.text) {
        part->dirty = true;
        part->text = new_part.text;
        delete part->text_layout;
        part->text_layout =
            text_format_->CreateLayout(part->text, big_size).release();
        DWRITE_TEXT_METRICS text_metrics;
        COM_VERIFY((*part->text_layout)->GetMetrics(&text_metrics));
        part->width = std::max(text_metrics.width + 8, ::ceil(part->width));
        dirty = true;
      }
      total_width += part->width;
      ++part;
    }
    auto const main_part_width = std::max(
        new_bounds.width() - total_width + parts_[0].width, 1.0f);
    if (parts_[0].width != main_part_width) {
      parts_[0].width = main_part_width;
      parts_[0].dirty = true;
      dirty = true;
    }
  }

  // Update part bounds
  {
    auto origin = new_bounds.origin();
    for (auto& part : parts_) {
      auto const new_part_bounds = gfx::RectF(origin,
          gfx::SizeF(part.width, new_bounds.height()));
      DCHECK(!new_part_bounds.empty());
      if (part.bounds != new_part_bounds) {
        part.bounds = new_part_bounds;
        part.dirty = true;
        dirty = true;
      }
      origin += gfx::SizeF(part.width, 0.0f);
    }
  }

  if (!dirty)
    return;

  gfx::Canvas::DrawingScope drawing_scope(canvas);
  auto const alpha = 1.0f;
  if (bounds_ != new_bounds) {
    canvas->Clear(gfx::sysColor(COLOR_BTNFACE, alpha));
    for (auto& part : parts_) {
      part.dirty = true;
    }
    bounds_ = new_bounds;
  }

  // Paint parts
  gfx::Brush text_brush(canvas, gfx::sysColor(COLOR_BTNTEXT));
  for (auto& part : parts_) {
    if (!part.dirty)
      continue;
    part.dirty = false;
    canvas->AddDirtyRect(part.bounds);
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, part.bounds);
    if (!part.text_layout)
      continue;
    canvas->Clear(gfx::sysColor(COLOR_BTNFACE, alpha));
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
  private: base::string16 main_text_;
  private: base::Time main_text_time_;

  public: Model();
  public: ~Model() = default;

  public: void SetMessage(const base::string16& text);
  public: void SetStatus(const std::vector<base::string16>& texts);
  private: void UpdateLayout();
  public: void UpdateView(gfx::Canvas* canvas, const gfx::RectF& bounds);

  DISALLOW_COPY_AND_ASSIGN(Model);
};

MessageView::Model::Model() : paint_model_(new ModelView()), parts_(1) {
}

void MessageView::Model::SetMessage(const base::string16& text) {
  parts_[0].text = text;
}

void MessageView::Model::SetStatus(const std::vector<base::string16>& texts) {
  DCHECK(!texts.empty());
  parts_.clear();
  for (auto const text : texts){
    PartModel part_model {text};
    parts_.push_back(part_model);
  }
}

void MessageView::Model::UpdateView(gfx::Canvas* canvas,
                                    const gfx::RectF& bounds) {
  auto const now = base::Time::Now();
  if (main_text_ != parts_[0].text) {
    // TODO(eval1749) We should request animation frame for hiding main text
    // after 5 second.
    main_text_ = parts_[0].text;
    main_text_time_ = now;
  } else if (now - main_text_time_ >= base::TimeDelta::FromSeconds(5)) {
    // Hide message shown more than 5 second
    parts_[0].text.clear();
  }
  paint_model_->Paint(canvas, bounds, parts_);
  parts_[0].text = main_text_;
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
  if (!canvas_)
    return;
  model_->UpdateView(canvas_.get(), GetContentsBounds());
}

// ui::Widget
void MessageView::DidChangeBounds() {
  if (layer())
    layer()->SetBounds(gfx::RectF(bounds()));
  if (canvas_)
    canvas_->SetBounds(GetContentsBounds());
}

void MessageView::DidRealize() {
  auto const parent_layer = container_widget().layer();
  auto const layer = parent_layer->CreateLayer();
  SetLayer(layer);
  parent_layer->AppendChildLayer(layer);
  layer->SetBounds(gfx::RectF(bounds()));
  canvas_.reset(layer->CreateCanvas());
  DidChangeBounds();
}

}  // namespace views
