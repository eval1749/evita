// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_CANVAS_H_
#define EVITA_GFX_CANVAS_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

struct IDXGISwapChain1;

namespace gfx {

class Bitmap;
class CanvasObserver;
class SwapChain;

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
class Canvas : public Object, public DpiHandler {
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

  virtual ~Canvas();

  explicit operator bool() const { return GetRenderTarget() != nullptr; }
  operator ID2D1RenderTarget*() const { return GetRenderTarget(); }
  ID2D1RenderTarget* operator->() const { return GetRenderTarget(); }

  // Id of current backing bitmap. Each time we change canvas bounds, we have
  // new backing bitmap.
  int bitmap_id() const { return bitmap_id_; }
  // |drawing()| is for debugging.
  bool drawing() const { return batch_nesting_level_ != 0; }
  const FactorySet& factory_set() const { return *factory_set_; }
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
  virtual ID2D1RenderTarget* GetRenderTarget() const = 0;
  virtual bool IsReady() = 0;
  void RemoveObserver(CanvasObserver* observer);
  void RestoreScreenImage(const RectF& bounds);
  bool Canvas::SaveScreenImage(const RectF& bounds);
  void SetBounds(const RectF& bounds);
  void SetOffsetBounds(const gfx::RectF& bounds);

  // [T]
  bool TryAddDirtyRect(const gfx::RectF& bounds);

 protected:
  Canvas();

  virtual void AddDirtyRectImpl(const RectF& new_dirty_rect);
  virtual void DidCallEndDraw();
  void DidCreateRenderTarget();
  virtual void DidChangeBounds(const RectF& new_bounds) = 0;
  virtual void DidLostRenderTarget() = 0;
  void SetInitialBounds(const RectF& bounds);

 private:
  friend class DrawingScope;
  friend class ScopedState;

  void BeginDraw();
  void CommitDraw();
  void EndDraw();

  int batch_nesting_level_;
  int bitmap_id_;
  gfx::RectF bounds_;
  scoped_refptr<FactorySet> factory_set_;
  base::ObserverList<CanvasObserver> observers_;
  gfx::PointF offset_;
  std::unique_ptr<Bitmap> screen_bitmap_;
  bool should_clear_;

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

//////////////////////////////////////////////////////////////////////
//
// CanvasForHwnd
//
class CanvasForHwnd final : public Canvas {
 public:
  explicit CanvasForHwnd(HWND hwnd);
  ~CanvasForHwnd() final;

 private:
  // Implements |Canvas| member functions.
  void AddDirtyRectImpl(const RectF& new_dirty_rect) final;
  void DidCallEndDraw() final;
  void DidChangeBounds(const RectF& new_bounds) final;
  void DidLostRenderTarget() final;
  ID2D1RenderTarget* GetRenderTarget() const final;
  bool IsReady() final;

  HWND hwnd_;
  std::unique_ptr<SwapChain> swap_chain_;

  DISALLOW_COPY_AND_ASSIGN(CanvasForHwnd);
};

}  // namespace gfx

#endif  // EVITA_GFX_CANVAS_H_
