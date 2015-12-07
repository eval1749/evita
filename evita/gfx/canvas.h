// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_CANVAS_H_
#define EVITA_GFX_CANVAS_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

namespace gfx {

class Bitmap;
class CanvasObserver;
class CanvasOwner;
class SwapChain;

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
class Canvas final : public Object, public DpiHandler {
 public:
  class AxisAlignedClipScope final {
   public:
    AxisAlignedClipScope(Canvas* canvas,
                         const RectF& bounds,
                         D2D1_ANTIALIAS_MODE alias_mode);
    AxisAlignedClipScope(Canvas* canvas, const RectF& bounds);
    ~AxisAlignedClipScope();

   private:
    Canvas* canvas_;
    DISALLOW_COPY_AND_ASSIGN(AxisAlignedClipScope);
  };

  class DrawingScope final {
   public:
    explicit DrawingScope(Canvas* canvas);
    ~DrawingScope();

   private:
    Canvas* const canvas_;
    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };

  class ScopedState final {
   public:
    explicit ScopedState(Canvas* canvas);
    ~ScopedState();

   private:
    gfx::RectF bounds_;
    Canvas* const canvas_;
    gfx::PointF offset_;
    D2D1::Matrix3x2F transform_;

    DISALLOW_COPY_AND_ASSIGN(ScopedState);
  };

  explicit Canvas(CanvasOwner* owner);
  virtual ~Canvas();

  explicit operator bool() const { return GetRenderTarget() != nullptr; }
  operator ID2D1RenderTarget*() const { return GetRenderTarget(); }
  ID2D1RenderTarget* operator->() const { return GetRenderTarget(); }

  // Id of current backing bitmap. Each time we change canvas bounds, we have
  // new backing bitmap.
  int bitmap_id() const { return bitmap_id_; }
  // |drawing()| is for debugging.
  bool drawing() const { return batch_nesting_level_ != 0; }
  float height() const { return bounds_.height(); }
  Bitmap* screen_bitmap() const { return screen_bitmap_.get(); }
  bool should_clear() const { return should_clear_; }
  float width() const { return bounds_.width(); }

  virtual void AddDirtyRect(const RectF& new_dirty_rect);
  void AddObserver(CanvasObserver* observer);
  void Clear(const ColorF& color);
  void Clear(D2D1::ColorF::Enum name);
  void DrawBitmap(const Bitmap& bitmap,
                  const RectF& dst_rect,
                  const RectF& src_rect,
                  float opacity = 1.0f,
                  D2D1_BITMAP_INTERPOLATION_MODE mode =
                      D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
  void DrawLine(const Brush& brush,
                const PointF& point1,
                const PointF& point2,
                float pen_width = 1.0f);
  void DrawRectangle(const Brush& brush,
                     const RectF& rect,
                     float strokeWidth = 1);
  void DrawText(const TextFormat& text_format,
                const Brush& brush,
                const RectF& bounds,
                const base::string16& text);
  void FillRectangle(const Brush& brush, const RectF& rect);
  void Flush();
  gfx::RectF GetLocalBounds() const;
  common::ComPtr<ID2D1Factory> GetD2D1Factory() const;
  ID2D1RenderTarget* GetRenderTarget() const;
  bool IsReady() const;
  void RemoveObserver(CanvasObserver* observer);
  void RestoreScreenImage(const RectF& bounds);
  bool Canvas::SaveScreenImage(const RectF& bounds);
  void SetBounds(const RectF& bounds);
  void SetOffsetBounds(const gfx::RectF& bounds);

  // [T]
  bool TryAddDirtyRect(const gfx::RectF& bounds);

 private:
  friend class DrawingScope;
  friend class ScopedState;

  void BeginDraw();
  void CommitDraw();
  void DidLostRenderTarget();
  void EndDraw();

  int batch_nesting_level_;
  int bitmap_id_;
  gfx::RectF bounds_;
  base::ObserverList<CanvasObserver> observers_;
  gfx::PointF offset_;
  CanvasOwner* const owner_;
  std::unique_ptr<Bitmap> screen_bitmap_;
  bool should_clear_;
  std::unique_ptr<SwapChain> swap_chain_;

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

}  // namespace gfx

#endif  // EVITA_GFX_CANVAS_H_
