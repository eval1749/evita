// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_
#define EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_

#include <iosfwd>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/base/castable.h"
#include "evita/gfx/base/colors/float_color.h"
#include "evita/gfx/base/geometry/float_matrix3x2.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/visuals/display/public/display_items_forward.h"
#include "evita/visuals/fonts/text_layout.h"
#include "evita/visuals/imaging/image_bitmap.h"

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
class DisplayItem : public base::Castable<DisplayItem> {
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

// See "display_item_printers.cc" for implementation.
std::ostream& operator<<(std::ostream& ostream, const DisplayItem& item);
std::ostream& operator<<(std::ostream& ostream, const DisplayItem* item);

//////////////////////////////////////////////////////////////////////
//
// BeginClipDisplayItem
//
class BeginClipDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(BeginClipDisplayItem, DisplayItem)

 public:
  explicit BeginClipDisplayItem(const gfx::FloatRect& bounds);
  ~BeginClipDisplayItem();

  const gfx::FloatRect& bounds() const { return bounds_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatRect bounds_;

  DISALLOW_COPY_AND_ASSIGN(BeginClipDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// BeginTransformDisplayItem
//
class BeginTransformDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(BeginTransformDisplayItem, DisplayItem)

 public:
  explicit BeginTransformDisplayItem(const gfx::FloatMatrix3x2& matrix);
  ~BeginTransformDisplayItem();

  const gfx::FloatMatrix3x2& matrix() const { return matrix_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatMatrix3x2 matrix_;

  DISALLOW_COPY_AND_ASSIGN(BeginTransformDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// ClearDisplayItem - clears current clip rectangle with specified color.
//
class ClearDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(ClearDisplayItem, DisplayItem)

 public:
  explicit ClearDisplayItem(const gfx::FloatColor& color);
  ~ClearDisplayItem();

  const gfx::FloatColor& color() const { return color_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatColor color_;

  DISALLOW_COPY_AND_ASSIGN(ClearDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// DrawBitmapDisplayItem
//
class DrawBitmapDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(DrawBitmapDisplayItem, DisplayItem)

 public:
  DrawBitmapDisplayItem(const gfx::FloatRect& destination,
                        const ImageBitmap& bitmap,
                        const gfx::FloatRect& source,
                        float opacity);
  ~DrawBitmapDisplayItem();

  const ImageBitmap& bitmap() const { return bitmap_; }
  const gfx::FloatRect& destination() const { return destination_; }
  float opacity() const { return opacity_; }
  const gfx::FloatRect& source() const { return source_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const ImageBitmap bitmap_;
  const gfx::FloatRect destination_;
  const float opacity_;
  const gfx::FloatRect source_;

  DISALLOW_COPY_AND_ASSIGN(DrawBitmapDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// DrawLineDisplayItem
//
class DrawLineDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(DrawLineDisplayItem, DisplayItem)

 public:
  DrawLineDisplayItem(const gfx::FloatPoint& point1,
                      const gfx::FloatPoint& point2,
                      const gfx::FloatColor& color,
                      float thickness);
  ~DrawLineDisplayItem();

  const gfx::FloatColor& color() const { return color_; }
  const gfx::FloatPoint& point1() const { return point1_; }
  const gfx::FloatPoint& point2() const { return point2_; }
  float thickness() const { return thickness_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatColor color_;
  const gfx::FloatPoint point1_;
  const gfx::FloatPoint point2_;
  const float thickness_;

  DISALLOW_COPY_AND_ASSIGN(DrawLineDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// DrawRectDisplayItem
//
class DrawRectDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(DrawRectDisplayItem, DisplayItem)

 public:
  DrawRectDisplayItem(const gfx::FloatRect& bounds,
                      const gfx::FloatColor& color,
                      float thickness);
  ~DrawRectDisplayItem();

  const gfx::FloatRect& bounds() const { return bounds_; }
  const gfx::FloatColor& color() const { return color_; }
  float thickness() const { return thickness_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatRect bounds_;
  const gfx::FloatColor color_;
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
  DrawTextDisplayItem(const gfx::FloatRect& bounds,
                      const gfx::FloatColor& color,
                      float baseline,
                      const TextLayout& text_layout,
                      const base::string16& text);
  ~DrawTextDisplayItem();

  float baseline() const { return baseline_; }
  const gfx::FloatRect& bounds() const { return bounds_; }
  const gfx::FloatColor& color() const { return color_; }
  const base::string16& text() const { return text_; }
  const TextLayout& text_layout() const { return text_layout_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const float baseline_;
  const gfx::FloatRect bounds_;
  const gfx::FloatColor color_;
  // |text_| is used only for debugging.
  const base::string16 text_;
  const TextLayout text_layout_;

  DISALLOW_COPY_AND_ASSIGN(DrawTextDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// EndClipDisplayItem
//
class EndClipDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(EndClipDisplayItem, DisplayItem)

 public:
  EndClipDisplayItem();
  ~EndClipDisplayItem();

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  DISALLOW_COPY_AND_ASSIGN(EndClipDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// EndTransformDisplayItem
//
class EndTransformDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(EndTransformDisplayItem, DisplayItem)

 public:
  EndTransformDisplayItem();
  ~EndTransformDisplayItem();

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  DISALLOW_COPY_AND_ASSIGN(EndTransformDisplayItem);
};

//////////////////////////////////////////////////////////////////////
//
// FillRectDisplayItem
//
class FillRectDisplayItem final : public DisplayItem {
  DECLARE_DISPLAY_ITEM_FINAL_CLASS(FillRectDisplayItem, DisplayItem)

 public:
  FillRectDisplayItem(const gfx::FloatRect& bounds,
                      const gfx::FloatColor& color);
  ~FillRectDisplayItem();

  const gfx::FloatRect& bounds() const { return bounds_; }
  const gfx::FloatColor& color() const { return color_; }

 private:
  // DisplayItem
  bool EqualsTo(const DisplayItem& other) const final;

  const gfx::FloatRect bounds_;
  const gfx::FloatColor color_;

  DISALLOW_COPY_AND_ASSIGN(FillRectDisplayItem);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DISPLAY_PUBLIC_DISPLAY_ITEMS_H_
