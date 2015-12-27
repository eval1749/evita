// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_
#define EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_

#include <iosfwd>

#include "base/strings/string16.h"
#include "common/castable.h"
#include "evita/visuals/display/public/display_items_forward.h"
#include "evita/visuals/style/float_color.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

#define DECLARE_DISPLAY_ITEM_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)           \
  friend class DisplayItemEditor;

#define DECLARE_DISPLAY_ITEM_ABSTRACT_CLASS(self, super) \
  DECLARE_DISPLAY_ITEM_CLASS(self, super)

#define DECLARE_DISPLAY_ITEM_FINAL_CLASS(self, super) \
  DECLARE_DISPLAY_ITEM_CLASS(self, super)             \
 private:                                             \
  void Accept(DisplayItemVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// DisplayItem
//
class DisplayItem : public common::Castable {
  DECLARE_DISPLAY_ITEM_ABSTRACT_CLASS(DisplayItem, Castable)

 public:
  virtual ~DisplayItem();

  bool operator==(const DisplayItem& other) const;
  bool operator!=(const DisplayItem& other) const;

  int id() const { return id_; }

  virtual void Accept(DisplayItemVisitor* visitor) = 0;
  virtual bool EqualsTo(const DisplayItem& other) const = 0;

 protected:
  DisplayItem();

 private:
  const int id_;

  DISALLOW_COPY_AND_ASSIGN(DisplayItem);
};

std::ostream& operator<<(std::ostream& ostream, const DisplayItem& item);
std::ostream& operator<<(std::ostream& ostream, const DisplayItem* item);

//////////////////////////////////////////////////////////////////////
//
// BeginBoxDisplayItem
//
class BeginBoxDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(BeginBoxDisplayItem, DisplayItem)

 public:
  BeginBoxDisplayItem(int box_id, const FloatRect& bounds);
  ~BeginBoxDisplayItem();

  int box_id() const { return box_id_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const FloatRect bounds_;
  const int box_id_;

  DISALLOW_COPY_AND_ASSIGN(BeginBoxDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// DrawRectDisplayItem
//
class DrawRectDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(DrawRectDisplayItem, DisplayItem)

 public:
  DrawRectDisplayItem(const FloatRect& bounds,
                      const FloatColor& color,
                      float thickness);
  ~DrawRectDisplayItem();

  const FloatRect& bounds() const { return bounds_; }
  const FloatColor& color() const { return color_; }
  float thickness() const { return thickness_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const FloatRect bounds_;
  const FloatColor color_;
  const float thickness_;

  DISALLOW_COPY_AND_ASSIGN(DrawRectDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// DrawTextDisplayItem
//
class DrawTextDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(DrawTextDisplayItem, DisplayItem)

 public:
  DrawTextDisplayItem(const FloatRect& bounds,
                      const FloatColor& color,
                      float baseline,
                      const base::string16& text);
  ~DrawTextDisplayItem();

  float baseline() const { return baseline_; }
  const FloatRect& bounds() const { return bounds_; }
  const FloatColor& color() const { return color_; }
  const base::string16& text() const { return text_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const float baseline_;
  const FloatRect bounds_;
  const FloatColor color_;
  const base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(DrawTextDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// EndBoxDisplayItem
//
class EndBoxDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(EndBoxDisplayItem, DisplayItem)

 public:
  explicit EndBoxDisplayItem(int box_id);
  ~EndBoxDisplayItem();

  int box_id() const { return box_id_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const int box_id_;

  DISALLOW_COPY_AND_ASSIGN(EndBoxDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// FillRectDisplayItem
//
class FillRectDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(FillRectDisplayItem, DisplayItem)

 public:
  FillRectDisplayItem(const FloatRect& bounds, const FloatColor& color);
  ~FillRectDisplayItem();

  const FloatRect& bounds() const { return bounds_; }
  const FloatColor& color() const { return color_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const FloatRect bounds_;
  const FloatColor color_;

  DISALLOW_COPY_AND_ASSIGN(FillRectDisplayItem);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_
