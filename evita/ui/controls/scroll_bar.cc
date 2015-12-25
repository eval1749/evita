// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>

#include "evita/ui/controls/scroll_bar.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/stroke_style.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/ui/events/event.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::Data
//
ScrollBar::Data::Data()
    : minimum(0), maximum(0), thumb_value(0), thumb_size(0) {}

bool ScrollBar::Data::operator==(const Data& other) const {
  return minimum == other.minimum && maximum == other.maximum &&
         thumb_value == other.thumb_value && thumb_size == other.thumb_size;
}

bool ScrollBar::Data::operator!=(const Data& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::Location
//
enum class ScrollBar::Location {
  None,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  EastOfThumb,
  NorthOfThumb,
  SouthOfThumb,
  WestOfThumb,
  Thumb,
};

std::ostream& operator<<(std::ostream& ostream, ScrollBar::Location location) {
  return ostream << static_cast<int>(location);
}

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::HitTestResult
//
class ScrollBar::HitTestResult final {
 public:
  HitTestResult(Location location, const Part* part);
  HitTestResult(const HitTestResult& other);
  HitTestResult();
  ~HitTestResult() = default;

  explicit operator bool() const { return part_ != nullptr; }

  Location location() const { return location_; }
  Part* part() const { return part_; }

  Location location_;
  Part* part_;
};

ScrollBar::HitTestResult::HitTestResult(Location location, const Part* part)
    : location_(location), part_(const_cast<Part*>(part)) {
  if (part_) {
    DCHECK_NE(location_, Location::None);
  } else {
    DCHECK_EQ(location_, Location::None);
  }
}

ScrollBar::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.location_, other.part_) {}

ScrollBar::HitTestResult::HitTestResult()
    : HitTestResult(Location::None, nullptr) {}

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::Part
//
class ScrollBar::Part {
 public:
  enum class State {
    Active,
    Disabled,
    Hover,
    Normal,
  };

  virtual ~Part() = default;

  const gfx::RectF& bounds() const { return bounds_; }
  bool is_active() const { return state_ == State::Active; }
  bool is_disabled() const { return state_ == State::Disabled; }
  bool is_normal() const { return state_ == State::Normal; }
  State state() const { return state_; }
  void set_state(State new_state);

  virtual int GetValueAt(const gfx::PointF& point) const;
  virtual HitTestResult HitTest(const gfx::PointF& point) const = 0;
  virtual bool IsDirty() const;
  virtual void Render(gfx::Canvas* canvas, const gfx::RectF& bounds);
  virtual void ResetView();
  virtual void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                            const ScrollBar::Data& data) = 0;

 protected:
  typedef ScrollBar::HitTestResult HitTestResult;
  typedef ScrollBar::Location Location;

  Part();

  void set_bounds(const gfx::RectF& bounds) { new_bounds_ = bounds; }
  void set_current_bounds() { bounds_ = new_bounds_; }
  const gfx::RectF& new_bounds() const { return new_bounds_; }
  State new_state() const { return new_state_; }
  void set_current_state() { state_ = new_state_; }

  virtual void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const = 0;

 private:
  gfx::RectF bounds_;
  gfx::RectF new_bounds_;
  State new_state_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Part);
};

std::ostream& operator<<(std::ostream& ostream, ScrollBar::Part::State state) {
  return ostream << static_cast<int>(state);
}

ScrollBar::Part::Part() : new_state_(State::Normal), state_(State::Normal) {}

void ScrollBar::Part::set_state(State new_state) {
  new_state_ = new_state;
}

int ScrollBar::Part::GetValueAt(const gfx::PointF&) const {
  NOTREACHED();
  return 0;
}

bool ScrollBar::Part::IsDirty() const {
  return bounds_ != new_bounds_ || state_ != new_state_;
}

void ScrollBar::Part::Render(gfx::Canvas* canvas, const gfx::RectF& bounds) {
  bool changed = false;
  if (bounds_ != new_bounds_) {
    bounds_ = new_bounds_;
    changed = true;
  }
  if (state_ != new_state_) {
    state_ = new_state_;
    changed = true;
  }
  if (!changed)
    return;
  Paint(canvas, bounds);
}

void ScrollBar::Part::ResetView() {
  bounds_ = gfx::RectF();
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// Arrow
//
class Arrow : public ScrollBar::Part {
 protected:
  enum class Direction {
    Down,
    Left,
    Right,
    Up,
  };

  Arrow() = default;
  ~Arrow() override = default;

  void PaintArrow(gfx::Canvas* canvas,
                  const gfx::RectF& bounds,
                  Direction direction) const;

 private:
  gfx::ColorF bgcolor() const;
  gfx::ColorF color() const;

  DISALLOW_COPY_AND_ASSIGN(Arrow);
};

gfx::ColorF Arrow::bgcolor() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_BTNFACE, 1.0f);
    case State::Disabled:
      return gfx::sysColor(COLOR_BTNFACE, 0.3f);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNFACE, 0.6f);
    case State::Normal:
      return gfx::sysColor(COLOR_BTNFACE, 0.3f);
    default:
      NOTREACHED();
      return gfx::ColorF(gfx::ColorF::Red);
  }
}

gfx::ColorF Arrow::color() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_BTNTEXT, 1.0f);
    case State::Disabled:
      return gfx::sysColor(COLOR_BTNTEXT, 0.3f);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNTEXT, 0.6f);
    case State::Normal:
      return gfx::sysColor(COLOR_BTNTEXT, 0.3f);
    default:
      NOTREACHED();
      return gfx::ColorF(gfx::ColorF::Red);
  }
}

void Arrow::PaintArrow(gfx::Canvas* canvas,
                       const gfx::RectF& canvas_bounds,
                       Direction direction) const {
  float factors[4] = {0.0f};
  switch (direction) {
    case Direction::Down:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = -1.0f;
      break;
    case Direction::Left:
      factors[0] = 1.0f;
      factors[1] = -1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Right:
      factors[0] = -1.0f;
      factors[1] = -1.0f;
      factors[2] = -1.0f;
      factors[3] = 1.0f;
      break;
    case Direction::Up:
      factors[0] = -1.0f;
      factors[1] = 1.0f;
      factors[2] = 1.0f;
      factors[3] = 1.0f;
      break;
    default:
      NOTREACHED();
      break;
  }

  auto const bounds = this->bounds().Offset(canvas_bounds.origin());
  canvas->AddDirtyRect(bounds);

  auto const center_x = bounds.left + bounds.width() / 2;
  auto const center_y = bounds.top + bounds.height() / 2;
  auto const wing_size = ::floor(bounds.width() / 4);
  auto const pen_width = 2.0f;

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->Clear(gfx::sysColor(COLOR_BTNFACE));
  canvas->FillRectangle(gfx::Brush(canvas, bgcolor()), bounds);

  // TODO(eval1749): We should have only one arrow figure as singleton and
  // paint with rotation for all directions.
  common::ComPtr<ID2D1PathGeometry> geometry;
  canvas->GetD2D1Factory()->CreatePathGeometry(&geometry);
  {
    common::ComPtr<ID2D1GeometrySink> sink;
    geometry->Open(&sink);
    sink->BeginFigure(gfx::PointF(center_x + factors[0] * wing_size,
                                  center_y + factors[1] * wing_size),
                      D2D1_FIGURE_BEGIN_HOLLOW);
    sink->AddLine(gfx::PointF(center_x, center_y));
    sink->AddLine(gfx::PointF(center_x + factors[2] * wing_size,
                              center_y + factors[3] * wing_size));

    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
  }

  // TODO(eval1749): Arrow stroke style should be singleton. Stroke style is
  // a device independent resource.
  gfx::StrokeStyle stroke_style;
  stroke_style.set_cap_style(gfx::CapStyle::Flat);
  stroke_style.set_line_join(gfx::LineJoin::Miter);
  stroke_style.Realize(canvas);

  gfx::Brush arrow_brush(canvas, color());
  (*canvas)->DrawGeometry(geometry, arrow_brush, pen_width, stroke_style);
}

//////////////////////////////////////////////////////////////////////
//
// ArrowDown
//
class ArrowDown final : public Arrow {
 public:
  ArrowDown() = default;
  ~ArrowDown() final = default;

 private:
  // ScrollBar::Part
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ArrowDown);
};

ScrollBar::HitTestResult ArrowDown::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(Location::ArrowDown, this);
}

void ArrowDown::Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const {
  PaintArrow(canvas, bounds, Direction::Down);
}

void ArrowDown::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                             const ScrollBar::Data& data) {
  set_state(data.thumb_value <= data.minimum ? State::Disabled : State::Normal);
  auto const size = scroll_bar_bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(scroll_bar_bounds.left, scroll_bar_bounds.bottom - size),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowLeft
//
class ArrowLeft final : public Arrow {
 public:
  ArrowLeft() = default;
  ~ArrowLeft() final = default;

 private:
  // ScrollBar::Part
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ArrowLeft);
};

ScrollBar::HitTestResult ArrowLeft::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(Location::ArrowLeft, this);
}

void ArrowLeft::Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const {
  PaintArrow(canvas, bounds, Direction::Left);
}

void ArrowLeft::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                             const ScrollBar::Data& data) {
  set_state(data.thumb_value <= data.minimum ? State::Disabled : State::Normal);
  auto const size = scroll_bar_bounds.width();
  set_bounds(gfx::RectF(scroll_bar_bounds.origin(), gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowRight
//
class ArrowRight final : public Arrow {
 public:
  ArrowRight() = default;
  ~ArrowRight() final = default;

 private:
  // ScrollBar::Part
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ArrowRight);
};

ScrollBar::HitTestResult ArrowRight::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(Location::ArrowRight, this);
}

void ArrowRight::Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const {
  PaintArrow(canvas, bounds, Direction::Right);
}

void ArrowRight::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                              const ScrollBar::Data& data) {
  set_state(data.thumb_value + data.thumb_size >= data.minimum ? State::Disabled
                                                               : State::Normal);
  auto const size = scroll_bar_bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(scroll_bar_bounds.right - size, scroll_bar_bounds.top),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowUp
//
class ArrowUp final : public Arrow {
 public:
  ArrowUp() = default;
  ~ArrowUp() final = default;

 private:
  // ScrollBar::Part
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ArrowUp);
};

ScrollBar::HitTestResult ArrowUp::HitTest(const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  return HitTestResult(Location::ArrowUp, this);
}

void ArrowUp::Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const {
  PaintArrow(canvas, bounds, Direction::Up);
}

void ArrowUp::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                           const ScrollBar::Data& data) {
  set_state(data.thumb_value + data.thumb_size >= data.minimum ? State::Disabled
                                                               : State::Normal);
  auto const size = scroll_bar_bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(scroll_bar_bounds.right - size, scroll_bar_bounds.top),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// Thumb
//
class Thumb : public ScrollBar::Part {
 protected:
  Thumb() = default;
  ~Thumb() override = default;

  const ScrollBar::Data& data() const { return data_; }
  void set_data(const ScrollBar::Data& data) { data_ = data; }
  const gfx::RectF thumb_bounds() const { return thumb_bounds_; }
  void set_thumb_bounds(const gfx::RectF& new_thumb_bounds) {
    new_thumb_bounds_ = new_thumb_bounds;
  }

  float ComputeThumbSize(float screen_size) const;

 private:
  gfx::ColorF thumb_color() const;

  void PaintThumb(gfx::Canvas* canvas, const gfx::RectF& bounds) const;

  // ScrollBar::Part
  bool IsDirty() const override;
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) const override;
  void Render(gfx::Canvas* canvas, const gfx::RectF& bounds) override;
  void ResetView() override;

  ScrollBar::Data data_;
  gfx::RectF new_thumb_bounds_;
  gfx::RectF thumb_bounds_;

  DISALLOW_COPY_AND_ASSIGN(Thumb);
};

gfx::ColorF Thumb::thumb_color() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_BTNSHADOW, 0.7f);
    case State::Disabled:
      NOTREACHED();
      return gfx::ColorF(gfx::ColorF::Red);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNSHADOW, 0.5f);
    case State::Normal:
      return gfx::sysColor(COLOR_BTNSHADOW, 0.3f);
    default:
      NOTREACHED();
      return gfx::ColorF(gfx::ColorF::Red);
  }
}

float Thumb::ComputeThumbSize(float thumb_max_size) const {
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0)
    return 0.0f;

  if (data_.thumb_value <= data_.minimum &&
      data_.thumb_value + data_.thumb_size >= data_.maximum) {
    // Thumb covers all.
    return 0.0f;
  }

  auto const scale = thumb_max_size / size;
  auto const thumb_size = data_.thumb_size * scale;
  return thumb_size >= 1.0f ? scale : 0.0f;
}

void Thumb::PaintThumb(gfx::Canvas* canvas,
                       const gfx::RectF& canvas_bounds) const {
  if (thumb_bounds_.empty())
    return;
  auto const bounds = thumb_bounds_.Offset(canvas_bounds.origin());
  canvas->AddDirtyRect(bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->Clear(gfx::sysColor(COLOR_BTNFACE));
  canvas->FillRectangle(gfx::Brush(canvas, thumb_color()), bounds);
}

// ScrollBar::Part
void Thumb::Paint(gfx::Canvas* canvas, const gfx::RectF& canvas_bounds) const {
  auto const bounds = this->bounds().Offset(canvas_bounds.origin());
  canvas->AddDirtyRect(bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  canvas->Clear(gfx::sysColor(COLOR_BTNFACE));
  PaintThumb(canvas, canvas_bounds);
}

void Thumb::Render(gfx::Canvas* canvas, const gfx::RectF& canvas_bounds) {
  if (thumb_bounds_ != new_thumb_bounds_ || bounds() != new_bounds()) {
    thumb_bounds_ = new_thumb_bounds_;
    set_current_bounds();
    set_current_state();
    Paint(canvas, canvas_bounds);
    return;
  }

  if (!thumb_bounds_.empty() && state() != new_state()) {
    set_current_state();
    PaintThumb(canvas, canvas_bounds);
    return;
  }
}

bool Thumb::IsDirty() const {
  return Part::IsDirty() || thumb_bounds_ != new_thumb_bounds_;
}

void Thumb::ResetView() {
  Part::ResetView();
  thumb_bounds_ = gfx::RectF();
}

//////////////////////////////////////////////////////////////////////
//
// ThumbHorizontal
//
class ThumbHorizontal final : public Thumb {
 public:
  ThumbHorizontal() = default;
  ~ThumbHorizontal() final = default;

 private:
  // ScrollBar::Part
  int GetValueAt(const gfx::PointF& point) const final;
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ThumbHorizontal);
};

// ScrollBar::Part
int ThumbHorizontal::GetValueAt(const gfx::PointF& point) const {
  auto const size = data().maximum - data().minimum;
  if (size <= 0)
    return data().minimum;
  auto const offset =
      std::min(std::max(point.x - bounds().left, 0.0f), bounds().right);
  auto const scale = static_cast<float>(size) / bounds().width();
  return std::min(static_cast<int>(offset * scale) + data().minimum,
                  data().maximum);
}

ScrollBar::HitTestResult ThumbHorizontal::HitTest(
    const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  if (thumb_bounds().Contains(point))
    return HitTestResult(Location::Thumb, this);
  return HitTestResult(point.x < thumb_bounds().left ? Location::EastOfThumb
                                                     : Location::WestOfThumb,
                       this);
}

void ThumbHorizontal::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                                   const ScrollBar::Data& data) {
  auto const arrow_box_size = scroll_bar_bounds.height();
  auto const bounds = scroll_bar_bounds.Inset(arrow_box_size, 0.0f);
  set_bounds(bounds);
  set_data(data);

  auto const scale = ComputeThumbSize(bounds.width());
  if (!scale) {
    set_state(State::Disabled);
    set_thumb_bounds(gfx::RectF());
    return;
  }

  if (new_state() == State::Disabled)
    set_state(State::Normal);

  auto const thumb_left = ::floor((data.thumb_value - data.minimum) * scale);
  auto const thumb_right =
      ::floor((data.thumb_value + data.thumb_size - data.minimum) * scale);
  set_thumb_bounds(
      gfx::RectF(gfx::PointF(bounds.left + thumb_left, bounds.top),
                 gfx::PointF(bounds.left + thumb_right, bounds.bottom)));
}

//////////////////////////////////////////////////////////////////////
//
// ThumbVertical
//
class ThumbVertical final : public Thumb {
 public:
  ThumbVertical() = default;
  ~ThumbVertical() final = default;

 private:
  // ScrollBar::Part
  int GetValueAt(const gfx::PointF& point) const final;
  HitTestResult HitTest(const gfx::PointF& point) const final;
  void UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                    const ScrollBar::Data& data) final;

  DISALLOW_COPY_AND_ASSIGN(ThumbVertical);
};

// ScrollBar::Part
int ThumbVertical::GetValueAt(const gfx::PointF& point) const {
  auto const size = data().maximum - data().minimum;
  if (size <= 0)
    return data().minimum;
  auto const offset =
      std::min(std::max(point.y - bounds().top, 0.0f), bounds().bottom);
  auto const scale = size / bounds().height();
  return std::min(static_cast<int>(offset * scale) + data().minimum,
                  data().maximum);
}

ScrollBar::HitTestResult ThumbVertical::HitTest(
    const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  if (thumb_bounds().Contains(point))
    return HitTestResult(Location::Thumb, this);
  return HitTestResult(point.y < thumb_bounds().top ? Location::NorthOfThumb
                                                    : Location::SouthOfThumb,
                       this);
}

void ThumbVertical::UpdateLayout(const gfx::RectF& scroll_bar_bounds,
                                 const ScrollBar::Data& data) {
  auto const arrow_box_size = scroll_bar_bounds.width();
  auto const bounds = scroll_bar_bounds.Inset(0.0f, arrow_box_size);
  set_bounds(bounds);
  set_data(data);

  auto const scale = ComputeThumbSize(bounds.height());
  if (!scale) {
    set_state(State::Disabled);
    set_thumb_bounds(gfx::RectF());
    return;
  }

  if (new_state() == State::Disabled)
    set_state(State::Normal);

  auto const thumb_top = ::floor((data.thumb_value - data.minimum) * scale);
  auto const thumb_bottom =
      ::floor((data.thumb_value + data.thumb_size - data.minimum) * scale);
  set_thumb_bounds(
      gfx::RectF(gfx::PointF(bounds.left, bounds.top + thumb_top),
                 gfx::PointF(bounds.right, bounds.top + thumb_bottom)));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(Type type, ScrollBarObserver* observer)
    : capturing_location_(Location::None),
      hover_part_(nullptr),
      observer_(observer),
      parts_(CreateParts(type)) {}

ScrollBar::~ScrollBar() {
  for (auto part : parts_)
    delete part;
}

std::vector<ScrollBar::Part*> ScrollBar::CreateParts(Type type) {
  if (type == Type::Horizontal) {
    return std::vector<Part*>{
        new ArrowLeft(), new ThumbHorizontal(), new ArrowRight(),
    };
  }
  return std::vector<Part*>{
      new ArrowUp(), new ThumbVertical(), new ArrowDown(),
  };
}

ScrollBar::HitTestResult ScrollBar::HitTest(const gfx::PointF& point) const {
  for (auto part : parts_) {
    if (auto const result = part->HitTest(point))
      return result;
  }
  return HitTestResult();
}

void ScrollBar::ResetHover() {
  if (!hover_part_)
    return;
  if (capturing_location_ != Location::None) {
    ReleaseCapture();
    capturing_location_ = Location::None;
  }
  hover_part_->set_state(Part::State::Normal);
  hover_part_ = nullptr;
  SchedulePaint();
}

void ScrollBar::SetData(const Data& data) {
  if (data_ == data)
    return;
  data_ = data;
  UpdateLayout();
}

void ScrollBar::UpdateLayout() {
  auto const contents_bounds = GetContentsBounds();
  for (auto part : parts_)
    part->UpdateLayout(contents_bounds, data_);
  SchedulePaint();
}

// ui::Widget
void ScrollBar::DidChangeBounds() {
  ui::Widget::DidChangeBounds();
  for (auto part : parts_)
    part->ResetView();
  UpdateLayout();
}

void ScrollBar::DidShow() {
  ui::Widget::DidShow();
  for (auto part : parts_)
    part->ResetView();
}

void ScrollBar::OnDraw(gfx::Canvas* canvas) {
  auto dirty = false;
  for (auto part : parts_)
    dirty |= part->IsDirty();
  if (!dirty)
    return;
  auto const canvas_bounds = GetContentsBounds();
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, canvas_bounds);
  for (auto part : parts_)
    part->Render(canvas, canvas_bounds);
}

void ScrollBar::OnMouseExited(const ui::MouseEvent&) {
  ResetHover();
}

void ScrollBar::OnMouseMoved(const ui::MouseEvent& event) {
  if (capturing_location_ == Location::Thumb) {
    DCHECK(hover_part_);
    observer_->DidMoveThumb(
        hover_part_->GetValueAt(gfx::PointF(event.location())));
    return;
  }
  if (capturing_location_ != Location::None)
    return;
  auto const result = HitTest(gfx::PointF(event.location()));
  auto const new_hover_part = result.part();
  if (hover_part_ && hover_part_ != new_hover_part) {
    hover_part_->set_state(Part::State::Normal);
    SchedulePaint();
  }
  hover_part_ = new_hover_part;
  if (!hover_part_ || !hover_part_->is_normal())
    return;
  hover_part_->set_state(Part::State::Hover);
  SchedulePaint();
}

void ScrollBar::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  auto const result = HitTest(gfx::PointF(event.location()));
  auto const new_hover_part = result.part();
  if (hover_part_ != new_hover_part) {
    if (hover_part_)
      hover_part_->set_state(Part::State::Normal);
    hover_part_ = new_hover_part;
  }

  if (!hover_part_)
    return;

  hover_part_->set_state(Part::State::Active);
  capturing_location_ = result.location();
  SetCapture();

  switch (result.location()) {
    case Location::ArrowLeft:
    case Location::ArrowUp:
      observer_->DidClickLineUp();
      break;
    case Location::ArrowRight:
    case Location::ArrowDown:
      observer_->DidClickLineDown();
      break;
    case Location::EastOfThumb:
    case Location::NorthOfThumb:
      observer_->DidClickPageUp();
      break;
    case Location::None:
      break;
    case Location::SouthOfThumb:
    case Location::WestOfThumb:
      observer_->DidClickPageDown();
      break;
    case Location::Thumb:
      break;
  }
}

void ScrollBar::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button())
    return;
  ResetHover();
}

}  // namespace ui
