// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/scroll_bar.h"

#include "common/win/rect_ostream.h"
#include "evita/gfx_base.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/ui/events/event.h"

extern HINSTANCE g_hInstance;

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
// ScrollBar::Part
//
class ScrollBar::Part {
  public: enum class State {
    Active,
    Hover,
    HoverAbove,
    HoverBelow,
    Normal,
  };

  private: gfx::Rect bounds_;
  private: ScrollBarObserver* const observer_;
  private: State state_;

  protected: Part(ScrollBarObserver* observer);
  public: virtual ~Part();

  public: const gfx::Rect& bounds() const { return bounds_; }
  protected: void set_bounds(const gfx::Rect& bounds) { bounds_ = bounds; }
  public: bool is_active() const { return state_ == State::Active; }
  public: bool is_normal() const { return state_ == State::Normal; }
  public: ScrollBarObserver* observer() const { return observer_; }
  public: State state() const { return state_; }
  public: void set_state(State new_state) { state_ = new_state; }

  public: virtual void Draw(gfx::Canvas* canvas) const = 0;
  public: bool HitTest(const gfx::Point& point) const;
  public: virtual void OnMouseMoved(const ui::MouseEvent& event);
  public: virtual void OnMousePressed(const ui::MouseEvent& event) = 0;
  public: virtual void UpdateLayout(const gfx::Rect& bounds,
                                    const ScrollBar::Data& data) = 0;

  DISALLOW_COPY_AND_ASSIGN(Part);
};

std::ostream& operator<<(std::ostream& ostream, ScrollBar::Part::State state) {
  return ostream << static_cast<int>(state);
}

ScrollBar::Part::Part(ScrollBarObserver* observer)
    : observer_(observer), state_(State::Normal) {
}

ScrollBar::Part::~Part() {
}

bool ScrollBar::Part::HitTest(const gfx::Point& point) const {
  return bounds_.Contains(point);
}

void ScrollBar::Part::OnMouseMoved(const ui::MouseEvent&) {
}

namespace {

//////////////////////////////////////////////////////////////////////
//
// Arrow
//
class Arrow : public ScrollBar::Part {
  protected: Arrow(ScrollBarObserver* observer);
  protected: virtual ~Arrow();

  private: gfx::ColorF bgcolor() const;
  private: gfx::ColorF color() const;

  protected: void DrawArrow(gfx::Canvas* canvas, int x_factor,
                            int y_factor) const;

  DISALLOW_COPY_AND_ASSIGN(Arrow);
};

Arrow::Arrow(ScrollBarObserver* observer) : Part(observer) {
}

Arrow::~Arrow() {
}

gfx::ColorF Arrow::bgcolor() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_3DDKSHADOW);
    case State::Hover:
      return gfx::sysColor(COLOR_BTNSHADOW);
    case State::Normal:
      return gfx::sysColor(COLOR_BTNFACE);
    case State::HoverAbove:
    case State::HoverBelow:
    default:
      NOTREACHED();
      return gfx::sysColor(COLOR_BTNFACE);
  }
}

gfx::ColorF Arrow::color() const {
  return gfx::sysColor(COLOR_3DDKSHADOW);
}

void Arrow::DrawArrow(gfx::Canvas* canvas, int x_factor,
                      int y_factor) const {
  gfx::Brush fill_brush(*canvas, bgcolor());
  canvas->FillRectangle(fill_brush, bounds());
  gfx::Brush arrow_brush(*canvas, color());
  auto const margin = bounds().width() / 3;
  auto const pen_width = 2.0f;
  auto const center_x = bounds().left + bounds().width() / 2;
  auto const center_y = bounds().top + bounds().height() / 2;
  if (!y_factor) {
    DCHECK(x_factor == 1 || x_factor == -1);
    canvas->DrawLine(arrow_brush,
                     center_x + margin * x_factor, bounds().top + margin,
                     center_x, center_y, pen_width);
    canvas->DrawLine(arrow_brush,
                     center_x, center_y,
                     center_x + margin * x_factor, bounds().bottom - margin,
                     pen_width);
  } else {
    DCHECK_EQ(0, x_factor);
    DCHECK(y_factor == 1 || y_factor == -1);
    canvas->DrawLine(arrow_brush,
                     bounds().left + margin, center_y + margin * y_factor,
                     center_x, center_y, pen_width);
    canvas->DrawLine(arrow_brush,
                     bounds().right - margin, center_y + margin * y_factor,
                     center_x, center_y,
                     pen_width);
  }
}

//////////////////////////////////////////////////////////////////////
//
// ArrowDown
//
class ArrowDown : public Arrow {
  public: ArrowDown(ScrollBarObserver* observer);
  public: virtual ~ArrowDown();

  // ScrollBar::Part
  private: virtual void Draw(gfx::Canvas* canvas) const override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ArrowDown);
};

ArrowDown::ArrowDown(ScrollBarObserver* observer) : Arrow(observer) {
}

ArrowDown::~ArrowDown() {
}

void ArrowDown::Draw(gfx::Canvas* canvas) const {
  DrawArrow(canvas, 0, -1);
}

void ArrowDown::OnMousePressed(const ui::MouseEvent&) {
  observer()->DidClickLineDown();
}

void ArrowDown::UpdateLayout(const gfx::Rect& bounds,
                                        const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::Rect(
      gfx::Point(bounds.right - size, bounds.bottom - size),
      gfx::Size(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowLeft
//
class ArrowLeft : public Arrow {
  public: ArrowLeft(ScrollBarObserver* observer);
  public: virtual ~ArrowLeft();

  // ScrollBar::Part
  private: virtual void Draw(gfx::Canvas* canvas) const override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ArrowLeft);
};

ArrowLeft::ArrowLeft(ScrollBarObserver* observer) : Arrow(observer) {
}

ArrowLeft::~ArrowLeft() {
}

void ArrowLeft::Draw(gfx::Canvas* canvas) const {
  DrawArrow(canvas, 1, 0);
}

void ArrowLeft::OnMousePressed(const ui::MouseEvent&) {
  observer()->DidClickLineDown();
}

void ArrowLeft::UpdateLayout(const gfx::Rect& bounds,
                                        const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::Rect(bounds.left_top(), gfx::Size(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowRight
//
class ArrowRight : public Arrow {
  public: ArrowRight(ScrollBarObserver* observer);
  public: virtual ~ArrowRight();

  // ScrollBar::Part
  private: virtual void Draw(gfx::Canvas* canvas) const override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ArrowRight);
};

ArrowRight::ArrowRight(ScrollBarObserver* observer) : Arrow(observer) {
}

ArrowRight::~ArrowRight() {
}

void ArrowRight::Draw(gfx::Canvas* canvas) const {
  DrawArrow(canvas, 1, 0);
}

void ArrowRight::OnMousePressed(const ui::MouseEvent&) {
  observer()->DidClickLineDown();
}

void ArrowRight::UpdateLayout(const gfx::Rect& bounds,
                                         const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::Rect(
      gfx::Point(bounds.right - size, bounds.top),
      gfx::Size(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// ArrowUp
//
class ArrowUp : public Arrow {
  public: ArrowUp(ScrollBarObserver* observer);
  public: virtual ~ArrowUp();

  // ScrollBar::Part
  private: virtual void Draw(gfx::Canvas* canvas) const override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  DISALLOW_COPY_AND_ASSIGN(ArrowUp);
};

ArrowUp::ArrowUp(ScrollBarObserver* observer) : Arrow(observer) {
}

ArrowUp::~ArrowUp() {
}

void ArrowUp::Draw(gfx::Canvas* canvas) const {
  DrawArrow(canvas, 0, 1);
}

void ArrowUp::OnMousePressed(const ui::MouseEvent&) {
  observer()->DidClickLineUp();
}

void ArrowUp::UpdateLayout(const gfx::Rect& bounds,
                                      const ScrollBar::Data&) {
  auto const size = bounds.width();
  set_bounds(gfx::Rect(
      gfx::Point(bounds.right - size, bounds.top),
      gfx::Size(size, size)));
}

//////////////////////////////////////////////////////////////////////
//
// Thumb
//
class Thumb : public ScrollBar::Part {
  private: ScrollBar::Data data_;

  protected: Thumb(ScrollBarObserver* observer);
  protected: virtual ~Thumb();

  private: gfx::ColorF color() const;

  protected: virtual State ComputeStateFromPoint(
      const gfx::Point& point) const = 0;

  protected: virtual int ComputeValueFromPoint(
      const gfx::Point& point) const = 0;

  // ScrollBar::Part
  private: virtual void Draw(gfx::Canvas* canvas) const override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(Thumb);
};

Thumb::Thumb(ScrollBarObserver* observer) : Part(observer) {
}

Thumb::~Thumb() {
}

gfx::ColorF Thumb::color() const {
  switch (state()) {
    case State::Active:
      return gfx::sysColor(COLOR_SCROLLBAR, 1.0f);
    case State::Hover:
      return gfx::sysColor(COLOR_SCROLLBAR, 0.8f);
    case State::HoverAbove:
    case State::HoverBelow:
      return gfx::sysColor(COLOR_SCROLLBAR, 0.5f);
    case State::Normal:
      return gfx::sysColor(COLOR_SCROLLBAR, 0.3f);
    default:
      NOTREACHED();
      return gfx::sysColor(COLOR_SCROLLBAR, 0.3f);
  }
}

void Thumb::Draw(gfx::Canvas* canvas) const {
  if (bounds().empty())
    return;
  gfx::Brush fill_brush(*canvas, color());
  canvas->FillRectangle(fill_brush, bounds());
}

// ScrollBar::Part
void Thumb::OnMouseMoved(const ui::MouseEvent& event) {
  DCHECK(is_active());
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0)
    return;
  auto const value = ComputeValueFromPoint(event.location());
  observer()->DidMoveThumb(value);
}

void Thumb::OnMousePressed(const ui::MouseEvent& event) {
  if (bounds().empty() || state() == State::Active)
    return;
  auto const state = ComputeStateFromPoint(event.location());
  if (state == State::HoverAbove)
    observer()->DidClickPageUp();
  else if (state == State::HoverBelow)
    observer()->DidClickPageDown();
}

//////////////////////////////////////////////////////////////////////
//
// ThumbHorizontal
//
class ThumbHorizontal : public Thumb {
  private: ScrollBar::Data data_;

  public: ThumbHorizontal(ScrollBarObserver* observer);
  public: virtual ~ThumbHorizontal();

  // ScrollBar::Part
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  // Thumb
  private: virtual State ComputeStateFromPoint(
      const gfx::Point& point) const override;
  private: virtual int ComputeValueFromPoint(
      const gfx::Point& point) const override;

  DISALLOW_COPY_AND_ASSIGN(ThumbHorizontal);
};

ThumbHorizontal::ThumbHorizontal(ScrollBarObserver* observer)
    : Thumb(observer) {
}

ThumbHorizontal::~ThumbHorizontal() {
}

// ScrollBar::Part
void ThumbHorizontal::UpdateLayout(const gfx::Rect& bounds,
                                   const ScrollBar::Data& data) {
  data_ = data;
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0) {
    set_bounds(gfx::Rect());
    return;
  }

  auto const arrow_width = bounds.height();
  auto const scale = static_cast<float>(
      bounds.width() - arrow_width * 2) / size;
  auto const thumb_width = static_cast<int>(data_.thumb_size * scale);
  if (!thumb_width) {
    set_bounds(gfx::Rect());
    return;
  }

  auto const thumb_offset = static_cast<int>(
      (data_.thumb_value - data_.minimum) * scale);
  set_bounds(gfx::Rect(
      gfx::Point(bounds.left + arrow_width + thumb_offset, bounds.top),
      gfx::Size(thumb_width, bounds.height() - 1)));
}

// Thumb
ScrollBar::Part::State ThumbHorizontal::ComputeStateFromPoint(
    const gfx::Point& point) const {
  if (bounds().Contains(point))
    return State::Hover;
  return point.x < bounds().left ? State::HoverAbove : State::HoverBelow;
}

int ThumbHorizontal::ComputeValueFromPoint(const gfx::Point& point) const {
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

//////////////////////////////////////////////////////////////////////
//
// ThumbVertical
//
class ThumbVertical : public Thumb {
  private: ScrollBar::Data data_;

  public: ThumbVertical(ScrollBarObserver* observer);
  public: virtual ~ThumbVertical();

  // ScrollBar::Part
  private: virtual void UpdateLayout(const gfx::Rect& bounds,
                                     const ScrollBar::Data& data) override;

  // Thumb
  private: virtual State ComputeStateFromPoint(
      const gfx::Point& point) const override;
  private: virtual int ComputeValueFromPoint(
      const gfx::Point& point) const override;

  DISALLOW_COPY_AND_ASSIGN(ThumbVertical);
};

ThumbVertical::ThumbVertical(ScrollBarObserver* observer)
    : Thumb(observer) {
}

ThumbVertical::~ThumbVertical() {
}

// ScrollBar::Part
void ThumbVertical::UpdateLayout(const gfx::Rect& bounds,
                                 const ScrollBar::Data& data) {
  data_ = data;
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0) {
    set_bounds(gfx::Rect());
    return;
  }

  auto const arrow_height = bounds.width();
  auto const scale = static_cast<float>(
      bounds.height() - arrow_height * 2) / size;
  auto const thumb_height = static_cast<int>(data_.thumb_size * scale);
  if (!thumb_height) {
    set_bounds(gfx::Rect());
    return;
  }

  auto const thumb_offset = static_cast<int>(
      (data_.thumb_value - data_.minimum) * scale);
  set_bounds(gfx::Rect(
      gfx::Point(bounds.left, bounds.top + arrow_height + thumb_offset),
      gfx::Size(bounds.width() - 1, thumb_height)));
}

// Thumb
ScrollBar::Part::State ThumbVertical::ComputeStateFromPoint(
    const gfx::Point& point) const {
  if (bounds().Contains(point))
    return State::Hover;
  return point.y < bounds().top ? State::HoverAbove : State::HoverBelow;
}

int ThumbVertical::ComputeValueFromPoint(const gfx::Point& point) const {
  auto const size = data_.maximum - data_.minimum;
  if (size <= 0)
    return data_.minimum;
  auto const arrow_height = bounds().width();
  auto const offset = std::min(
      std::max(point.y - bounds().top - arrow_height, 0l),
      bounds().right);
  auto const scale = static_cast<float>(size) / bounds().height();
  return std::min(static_cast<int>(offset * scale) + data_.minimum,
                  data_.maximum);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ScrollBar
//
ScrollBar::ScrollBar(Type type, ScrollBarObserver* observer)
    : dirty_(true), hover_part_(nullptr),
      parts_(CreateParts(type, observer)), thumb_(parts_.back()) {
}

ScrollBar::~ScrollBar() {
  for (auto part : parts_) {
    delete part;
  }
}

// Note: thumb must be the last part.
std::vector<ScrollBar::Part*> ScrollBar::CreateParts(
    Type type, ScrollBarObserver* observer) {
  if (type == Type::Horizontal) {
    return std::vector<Part*> {
        new ArrowLeft(observer),
        new ArrowRight(observer),
        new ThumbHorizontal(observer),
    };
  }
  return std::vector<Part*> {
      new ArrowUp(observer),
      new ArrowDown(observer),
      new ThumbVertical(observer),
  };
}

ScrollBar::Part* ScrollBar::HitTest(const gfx::Point& point) const {
  for (auto part : parts_) {
    if (part->HitTest(point))
      return part;
  }
  return nullptr;
}

void ScrollBar::SetData(const Data& data) {
  if (data_ == data)
    return;
  data_ = data;
  UpdateLayout();
}

void ScrollBar::UpdateAppearance() {
  DCHECK(hover_part_);
  SchedulePaintInRect(hover_part_->bounds());
  dirty_ = true;
}

void ScrollBar::UpdateLayout() {
  dirty_ = true;
  for (auto part : parts_) {
    part->UpdateLayout(bounds(), data_);
  }
}

// ui::Widget
void ScrollBar::DidResize() {
  UpdateLayout();
}

void ScrollBar::DidShow() {
  dirty_ = true;
}

void ScrollBar::OnDraw(gfx::Canvas* canvas) {
  if (!dirty_)
    return;
  dirty_ = false;
  gfx::Canvas::DrawingScope drawing_scope(*canvas);
  canvas->set_dirty_rect(bounds());
  gfx::Brush bgcolor(*canvas, gfx::sysColor(COLOR_BTNFACE));
  canvas->FillRectangle(bgcolor, bounds());
  for (auto part : parts_) {
    part->Draw(canvas);
  }
}

void ScrollBar::OnMouseExited(const ui::MouseEvent&) {
  DVLOG(0) << "OnMouseExited " << hover_part_;
  if (!hover_part_)
    return;
  hover_part_->set_state(Part::State::Normal);
  UpdateAppearance();
  hover_part_ = nullptr;
}

void ScrollBar::OnMouseMoved(const ui::MouseEvent& event) {
  DVLOG(0) << "OnMouseMoved " << hover_part_;
  if (thumb_->is_active()) {
    thumb_->OnMouseMoved(event);
    return;
  }
  auto const new_hover_part = HitTest(event.location());
  if (hover_part_) {
    hover_part_->set_state(Part::State::Normal);
    UpdateAppearance();
  }
  hover_part_ = new_hover_part;
  if (!hover_part_) {
    if (thumb_->bounds().empty())
      return;
    if (thumb_->is_normal()) {
      thumb_->set_state(Part::State::HoverAbove);
      SchedulePaintInRect(thumb_->bounds());
    }
    return;
  }
  if (hover_part_->is_normal()) {
    hover_part_->set_state(Part::State::Hover);
    SchedulePaintInRect(hover_part_->bounds());
  }
}

void ScrollBar::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count())
    return;
  auto const part = HitTest(event.location());
  if (!part) {
    thumb_->OnMousePressed(event);
    return;
  }
  dirty_ = true;
  if (hover_part_ != part) {
    if (hover_part_) {
      UpdateAppearance();
      hover_part_->set_state(Part::State::Normal);
    }
    hover_part_ = part;
    UpdateAppearance();
    SetCapture();
  }
  part->set_state(Part::State::Active);
  part->OnMousePressed(event);
}

void ScrollBar::OnMouseReleased(const ui::MouseEvent& event) {
  if (!event.is_left_button())
    return;
  if (!hover_part_)
    return;
  hover_part_->set_state(Part::State::Normal);
  UpdateAppearance();
  hover_part_ = nullptr;
  ReleaseCapture();
}

}  // namespace ui
