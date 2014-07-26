// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/scroll_bar.h"

#include <ostream>

#include "common/win/rect_ostream.h"
#include "evita/gfx_base.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/ui/events/event.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::Data
//
ScrollBar::Data::Data()
    : minimum(0), maximum(0), thumb_value(0), thumb_size(0) {
}

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
class ScrollBar::HitTestResult {
  private: Location location_;
  private: Part* part_;

  public: HitTestResult(Location location, const Part* part);
  public: HitTestResult(const HitTestResult& other);
  public: HitTestResult();
  public: ~HitTestResult() = default;

  public: explicit operator bool() const { return part_; }

  public: Location location() const { return location_; }
  public: Part* part() const { return part_; }
};

ScrollBar::HitTestResult::HitTestResult(Location location, const Part* part)
    : location_(location), part_(const_cast<Part*>(part)) {
  if (part_)
    DCHECK_NE(location_, Location::None);
  else
    DCHECK_EQ(location_, Location::None);
}

ScrollBar::HitTestResult::HitTestResult(const HitTestResult& other)
    : HitTestResult(other.location_, other.part_) {
}

ScrollBar::HitTestResult::HitTestResult()
    : HitTestResult(Location::None, nullptr) {
}

//////////////////////////////////////////////////////////////////////
//
// ScrollBar::Part
//
class ScrollBar::Part {
  protected: typedef ScrollBar::HitTestResult HitTestResult;
  protected: typedef ScrollBar::Location Location;

  public: enum class State {
    Active,
    Disabled,
    Hover,
    Normal,
  };

  private: gfx::RectF bounds_;
  private: gfx::RectF new_bounds_;
  private: State new_state_;
  private: State state_;

  protected: Part();
  public: virtual ~Part() = default;

  public: const gfx::RectF& bounds() const { return bounds_; }
  protected: void set_bounds(const gfx::RectF& bounds) { new_bounds_ = bounds; }
  public: bool is_active() const { return state_ == State::Active; }
  public: bool is_normal() const { return state_ == State::Normal; }
  protected: const gfx::RectF& new_bounds() const { return new_bounds_; }
  protected: State new_state() const { return new_state_; }
  public: State state() const { return state_; }
  public: void set_state(State new_state);
  protected: void set_current_state() { state_ = new_state_; }

  public: virtual int GetValueAt(const gfx::PointF& point) const;
  public: virtual HitTestResult HitTest(const gfx::PointF& point) const = 0;
  public: virtual bool IsDirty() const;
  protected: virtual void Paint(gfx::Canvas* canvas,
                                const gfx::RectF& bounds) const = 0;
  public: virtual void Render(gfx::Canvas* canvas, const gfx::RectF& bounds);
  public: virtual void UpdateLayout(const gfx::RectF& bounds,
                                    const ScrollBar::Data& data) = 0;

  DISALLOW_COPY_AND_ASSIGN(Part);
};

std::ostream& operator<<(std::ostream& ostream, ScrollBar::Part::State state) {
  return ostream << static_cast<int>(state);
}

ScrollBar::Part::Part() : new_state_(State::Normal), state_(State::Normal) {
}

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

namespace {

//////////////////////////////////////////////////////////////////////
//
// Arrow
//
class Arrow : public ScrollBar::Part {
  protected: enum class Direction {
    Down,
    Left,
    Right,
    Up,
  };

  protected: Arrow() = default;
  protected: virtual ~Arrow() = default;

  private: gfx::ColorF bgcolor() const;
  private: gfx::ColorF color() const;

  protected: void PaintArrow(gfx::Canvas* canvas, const gfx::RectF& bounds,
                             Direction direction) const;

  DISALLOW_COPY_AND_ASSIGN(Arrow);
};

gfx::ColorF Arrow::bgcolor() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_BTNFACE, 0.1);
    case State::Disabled:
    case State::Normal:
      return gfx::sysColor(COLOR_BTNFACE);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNFACE, 0.5);
    default:
      NOTREACHED();
      return gfx::sysColor(COLOR_BTNFACE);
  }
}

gfx::ColorF Arrow::color() const {
  return gfx::sysColor(state() == State::Disabled ?  COLOR_GRAYTEXT :
                                                     COLOR_BTNTEXT);
}

void Arrow::PaintArrow(gfx::Canvas* canvas, const gfx::RectF& canvas_bounds,
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
   }

  auto const bounds = this->bounds().Offset(canvas_bounds.origin());
  canvas->AddDirtyRect(bounds);

  auto const center_x = bounds.left + bounds.width() / 2;
  auto const center_y = bounds.top + bounds.height() / 2;
  auto const wing_size = ::floor(bounds.width() / 3);
  auto const pen_width = 2.0f;

  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  (*canvas)->Clear(bgcolor());

  gfx::Brush arrow_brush(canvas, color());
  canvas->DrawLine(arrow_brush, center_x + factors[0] * wing_size,
                   center_y + factors[1] * wing_size,
                   center_x, center_y, pen_width);
  canvas->DrawLine(arrow_brush, center_x + factors[2] * wing_size,
                   center_y + factors[3] * wing_size,
                   center_x, center_y, pen_width);
}

//////////////////////////////////////////////////////////////////////
//
// ArrowDown
//
class ArrowDown : public Arrow {
  public: ArrowDown() = default;
  public: virtual ~ArrowDown() = default;

  // ScrollBar::Part
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Paint(gfx::Canvas* canvas,
                              const gfx::RectF& bounds) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

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

void ArrowDown::UpdateLayout(const gfx::RectF& bounds,
                             const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(bounds.right - size, bounds.bottom - size),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowLeft
//
class ArrowLeft : public Arrow {
  public: ArrowLeft() = default;
  public: virtual ~ArrowLeft() = default;

  // ScrollBar::Part
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Paint(gfx::Canvas* canvas,
                              const gfx::RectF& bounds) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

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

void ArrowLeft::UpdateLayout(const gfx::RectF& bounds,
                             const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::RectF(bounds.origin(), gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowRight
//
class ArrowRight : public Arrow {
  public: ArrowRight() = default;
  public: virtual ~ArrowRight() = default;

  // ScrollBar::Part
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Paint(gfx::Canvas* canvas,
                               const gfx::RectF& bounds) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

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

void ArrowRight::UpdateLayout(const gfx::RectF& bounds,
                              const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(bounds.right - size, bounds.top),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowUp
//
class ArrowUp : public Arrow {
  public: ArrowUp() = default;
  public: virtual ~ArrowUp() = default;

  // ScrollBar::Part
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void Paint(gfx::Canvas* canvas,
                               const gfx::RectF& bounds) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

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

void ArrowUp::UpdateLayout(const gfx::RectF& bounds,
                           const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::RectF(
      gfx::PointF(bounds.right - size, bounds.top),
      gfx::SizeF(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// Thumb
//
class Thumb : public ScrollBar::Part {
  private: ScrollBar::Data data_;
  private: gfx::RectF new_thumb_bounds_;
  private: gfx::RectF thumb_bounds_;

  protected: Thumb() = default;
  protected: virtual ~Thumb() = default;

  private: gfx::ColorF thumb_color() const;
  protected: const gfx::RectF thumb_bounds() const { return thumb_bounds_; }

  private: void PaintThumb(gfx::Canvas* canvas, const gfx::RectF& bounds) const;

  // ScrollBar::Part
  protected: virtual int GetValueAt(
      const gfx::PointF& point) const = 0;

  private: virtual bool IsDirty() const override;
  private: virtual void Paint(gfx::Canvas* canvas,
                               const gfx::RectF& bounds) const override;
  private: virtual void Render(gfx::Canvas* canvas,
                               const gfx::RectF& bounds) override;

  DISALLOW_COPY_AND_ASSIGN(Thumb);
};

gfx::ColorF Thumb::thumb_color() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_BTNTEXT, 0.8f);
    case State::Disabled:
      NOTREACHED();
      return gfx::sysColor(COLOR_BTNTEXT, 0.1f);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNTEXT, 0.5f);
    case State::Normal:
      return gfx::sysColor(COLOR_BTNTEXT, 0.3f);
    default:
      NOTREACHED();
      return gfx::sysColor(COLOR_BTNTEXT, 0.1f);
  }
}

void Thumb::PaintThumb(gfx::Canvas* canvas,
                       const gfx::RectF& canvas_bounds) const {
  if (thumb_bounds_.empty())
    return;
  auto const bounds = thumb_bounds_.Offset(canvas_bounds.origin());
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  (*canvas)->Clear(thumb_color());
}

// ScrollBar::Part
void Thumb::Paint(gfx::Canvas* canvas, const gfx::RectF& canvas_bounds) const {
  auto const bounds = this->bounds().Offset(canvas_bounds.origin());
  canvas->AddDirtyRect(bounds);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds);
  (*canvas)->Clear(gfx::sysColor(COLOR_BTNFACE));
  PaintThumb(canvas, bounds);
}

void Thumb::Render(gfx::Canvas* canvas, const gfx::RectF& canvas_bounds) {
  if (thumb_bounds_ != new_thumb_bounds_) {
    thumb_bounds_ = new_thumb_bounds_;
    Part::Render(canvas, canvas_bounds);
    return;
  }

  if (bounds() != new_bounds()) {
    Part::Render(canvas, canvas_bounds);
    return;
  }

  if (state() != new_state()) {
    set_current_state();
    canvas->AddDirtyRect(thumb_bounds_);
    PaintThumb(canvas, canvas_bounds);
    return;
  }
}

bool Thumb::IsDirty() const {
  return Part::IsDirty() || thumb_bounds_ != new_thumb_bounds_;
}

//////////////////////////////////////////////////////////////////////
//
// ThumbHorizontal
//
class ThumbHorizontal : public Thumb {
  private: ScrollBar::Data data_;

  public: ThumbHorizontal() = default;
  public: virtual ~ThumbHorizontal() = default;

  // ScrollBar::Part
  private: virtual int GetValueAt(const gfx::PointF& point) const override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ThumbHorizontal);
};

// ScrollBar::Part
int ThumbHorizontal::GetValueAt(const gfx::PointF& point) const {
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0)
    return data_.minimum;
  auto const arrow_width = bounds().height();
  auto const offset = std::min(
      std::max(static_cast<int>(point.x - bounds().left - arrow_width), 0),
      static_cast<int>(bounds().right));
  auto const width = bounds().width() - arrow_width * 2;
  auto const scale = static_cast<float>(size) / width;
  return std::min(static_cast<int>(offset * scale) + data_.minimum,
                  data_.maximum);
}

ScrollBar::HitTestResult ThumbHorizontal::HitTest(
    const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  if (thumb_bounds().Contains(point))
    return HitTestResult(Location::Thumb, this);
  return HitTestResult(point.x < thumb_bounds().left ? Location::EastOfThumb :
                                                       Location::WestOfThumb,
                       this);
}

void ThumbHorizontal::UpdateLayout(const gfx::RectF& bounds,
                                   const ScrollBar::Data& data) {
  data_ = data;
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0) {
    set_bounds(gfx::RectF());
    return;
  }

  auto const arrow_width = bounds.height();
  auto const scale = static_cast<float>(
      bounds.width() - arrow_width * 2) / size;
  auto const thumb_width = data_.thumb_size * scale;
  if (!thumb_width) {
    set_bounds(gfx::RectF());
    return;
  }

  auto const thumb_offset = (data_.thumb_value - data_.minimum) * scale;
  set_bounds(gfx::RectF(
      gfx::PointF(bounds.left + arrow_width + thumb_offset, bounds.top),
      gfx::SizeF(thumb_width, bounds.height() - 1)));
}

//////////////////////////////////////////////////////////////////////
//
// ThumbVertical
//
class ThumbVertical : public Thumb {
  private: ScrollBar::Data data_;

  public: ThumbVertical() = default;
  public: virtual ~ThumbVertical() = default;

  // ScrollBar::Part
  private: virtual int GetValueAt(const gfx::PointF& point) const override;
  private: virtual HitTestResult HitTest(
      const gfx::PointF& point) const override;
  private: virtual void UpdateLayout(const gfx::RectF& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ThumbVertical);
};

// ScrollBar::Part
int ThumbVertical::GetValueAt(const gfx::PointF& point) const {
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0)
    return data_.minimum;
  auto const arrow_height = bounds().width();
  auto const offset = std::min(
      std::max(point.y - bounds().top - arrow_height, 0.0f),
      bounds().right);
  auto const scale = size / bounds().height();
  return std::min(static_cast<int>(offset * scale) + data_.minimum,
                  data_.maximum);
}

ScrollBar::HitTestResult ThumbVertical::HitTest(
    const gfx::PointF& point) const {
  if (!bounds().Contains(point))
    return HitTestResult();
  if (thumb_bounds().Contains(point))
    return HitTestResult(Location::Thumb, this);
  return HitTestResult(point.y < thumb_bounds().top ? Location::NorthOfThumb :
                                                      Location::SouthOfThumb,
                       this);
}

void ThumbVertical::UpdateLayout(const gfx::RectF& bounds,
                                 const ScrollBar::Data& data) {
  data_ = data;
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0) {
    set_bounds(gfx::RectF());
    return;
  }

  auto const arrow_height = bounds.width();
  auto const scale = static_cast<float>(
      bounds.height() - arrow_height * 2) / size;
  auto const thumb_height = data_.thumb_size * scale;
  if (!thumb_height) {
    set_bounds(gfx::RectF());
    return;
  }

  auto const thumb_offset = (data_.thumb_value - data_.minimum) * scale;
  set_bounds(gfx::RectF(
      gfx::PointF(bounds.left, bounds.top + arrow_height + thumb_offset),
      gfx::SizeF(bounds.width() - 1, thumb_height)));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(Type type, ScrollBarObserver* observer)
    : capturing_part_(nullptr), hover_part_(nullptr), observer_(observer),
      parts_(CreateParts(type)) {
}

ScrollBar::~ScrollBar() {
  for (auto part : parts_) {
    delete part;
  }
}

std::vector<ScrollBar::Part*> ScrollBar::CreateParts(Type type) {
  if (type == Type::Horizontal) {
    return std::vector<Part*> {
        new ArrowLeft(),
        new ThumbHorizontal(),
        new ArrowRight(),
    };
  }
  return std::vector<Part*> {
      new ArrowUp(),
      new ThumbVertical(),
      new ArrowDown(),
  };
}

ScrollBar::HitTestResult ScrollBar::HitTest(const gfx::PointF& point) const {
  for (auto part : parts_) {
    if (auto const result = part->HitTest(point))
      return result;
  }
  return HitTestResult();
}

void ScrollBar::Render(gfx::Canvas* canvas) {
  auto dirty = false;
  for (auto part : parts_) {
    dirty |= part->IsDirty();
  }
  if (!dirty)
    return;
  auto const canvas_bounds = gfx::RectF(bounds());
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, canvas_bounds);
  for (auto part : parts_) {
    part->Render(canvas, canvas_bounds);
  }
}

void ScrollBar::SetData(const Data& data) {
  if (data_ == data)
    return;
  data_ = data;
  UpdateLayout();
}

void ScrollBar::UpdateLayout() {
  auto const contents_bounds = GetContentsBounds();
  for (auto part : parts_) {
    part->UpdateLayout(contents_bounds, data_);
  }
}

// ui::Widget
void ScrollBar::DidChangeBounds() {
  UpdateLayout();
}

void ScrollBar::OnMouseExited(const ui::MouseEvent&) {
  if (!hover_part_)
    return;
  hover_part_->set_state(Part::State::Normal);
  hover_part_ = nullptr;
}

void ScrollBar::OnMouseMoved(const ui::MouseEvent& event) {
  auto const point = gfx::PointF(event.location());
  auto const result = HitTest(event.location());
  if (hover_part_ && hover_part_ != result.part())
    hover_part_->set_state(Part::State::Normal);
  hover_part_ = result.part();
  if (!hover_part_)
    return;
  hover_part_->set_state(Part::State::Hover);
  if (result.location() == Location::Thumb) {
    observer_->DidMoveThumb(hover_part_->GetValueAt(point));
  }
}

void ScrollBar::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  DCHECK(!capturing_part_);
  auto const result = HitTest(gfx::PointF(event.location()));
  if (hover_part_ != result.part()) {
    if (hover_part_)
      hover_part_->set_state(Part::State::Normal);
    hover_part_ = result.part();
  }

  if (result.part())
    result.part()->set_state(Part::State::Active);

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
      if (result.location() == Location::Thumb) {
        capturing_part_ = result.part();
        SetCapture();
      }
      break;
  }

  if (result.location() == Location::NorthOfThumb ||
      result.location() == Location::EastOfThumb) {
  }
}

void ScrollBar::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button())
    return;
  if (hover_part_) {
    hover_part_->set_state(Part::State::Normal);
    hover_part_ = nullptr;
  }
  if (capturing_part_) {
    ReleaseCapture();
    capturing_part_ = nullptr;
  }
}

}  // namespace ui
