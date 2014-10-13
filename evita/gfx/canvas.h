// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_canvas_h)
#define INCLUDE_evita_gfx_canvas_h

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

interface IDXGISwapChain1;

namespace gfx {

class Bitmap;
class CanvasObserver;
class SwapChain;

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
class Canvas : public Object, public DpiHandler {
  public: class AxisAlignedClipScope final {
    private: Canvas* canvas_;
    public: AxisAlignedClipScope(Canvas* canvas, const RectF& bounds,
                                 D2D1_ANTIALIAS_MODE alias_mode);
    public: AxisAlignedClipScope(Canvas* canvas, const RectF& bounds);
    public: ~AxisAlignedClipScope();
    DISALLOW_COPY_AND_ASSIGN(AxisAlignedClipScope);
  };

  public: class DrawingScope final {
    private: Canvas* const canvas_;
    public: DrawingScope(Canvas* canvas);
    public: ~DrawingScope();
    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };
  friend class DrawingScope;

  public: class ScopedState final {
    private: gfx::RectF bounds_;
    private: Canvas* const canvas_;
    private: D2D1::Matrix3x2F transform_;

    public: ScopedState(Canvas* canvas);
    public: ~ScopedState();

    DISALLOW_COPY_AND_ASSIGN(ScopedState);
  };
  friend class ScopedState;

  private: int batch_nesting_level_;
  private: int bitmap_id_;
  private: gfx::RectF bounds_;
  private: scoped_refptr<FactorySet> factory_set_;
  private: ObserverList<CanvasObserver> observers_;
  private: std::unique_ptr<Bitmap> screen_bitmap_;
  private: bool should_clear_;

  protected: Canvas();
  public: virtual ~Canvas();

  public: explicit operator bool() const {
    return GetRenderTarget();
  }
  public: operator ID2D1RenderTarget*() const {
    return GetRenderTarget();
  }
  public: ID2D1RenderTarget* operator->() const {
    return GetRenderTarget();
  }

  // Id of current backing bitmap. Each time we change canvas bounds, we have
  // new backing bitmap.
  public: int bitmap_id() const { return bitmap_id_; }
  // |drawing()| is for debugging.
  public: bool drawing() const { return batch_nesting_level_; }
  public: const FactorySet& factory_set() const { return *factory_set_; }
  public: float height() const { return bounds_.height(); }
  public: Bitmap* screen_bitmap() const { return screen_bitmap_.get(); }
  public: bool should_clear() const { return should_clear_; }
  public: float width() const { return bounds_.width(); }

  // [A]
  public: virtual void AddDirtyRect(const RectF& new_dirty_rect);
  protected: virtual void AddDirtyRectImpl(const RectF& new_dirty_rect);
  public: void AddObserver(CanvasObserver* observer);

  // [B]
  private: void BeginDraw();

  // [C]
  public: void Clear(const ColorF& color);

  // [D]
  protected: virtual void DidCallEndDraw();
  protected: void DidCreateRenderTarget();
  protected: virtual void DidChangeBounds(const RectF& new_bounds) = 0;
  protected: virtual void DidLostRenderTarget() = 0;
  public: void DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity = 1.0f,
                          D2D1_BITMAP_INTERPOLATION_MODE mode =
                              D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
  public: void DrawLine(const Brush& brush, const PointF& point1,
                        const PointF& point2, float pen_width = 1.0f);
  public: void DrawRectangle(const Brush& brush, const RectF& rect,
                             float strokeWidth = 1);
  public: void DrawText(const TextFormat& text_format, const Brush& brush,
                        const RectF& bounds, const base::string16& text);

  // [E]
  private: void EndDraw();

  // [F]
  public: void FillRectangle(const Brush& brush, const RectF& rect);
  public: void Flush();

  // [G]
  public: gfx::RectF GetLocalBounds() const;
  public: virtual ID2D1RenderTarget* GetRenderTarget() const = 0;

  // [R]
  public: void RemoveObserver(CanvasObserver* observer);
  public: void RestoreScreenImage(const RectF& bounds);

  // [S]
  public: bool Canvas::SaveScreenImage(const RectF& bounds);
  public: void SetBounds(const RectF& bounds);
  protected: void SetInitialBounds(const RectF& bounds);
  public: void SetOffsetBounds(const gfx::RectF& bounds);

  // [T]
  public: bool TryAddDirtyRect(const gfx::RectF& bounds);

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

//////////////////////////////////////////////////////////////////////
//
// CanvasForHwnd
//
class CanvasForHwnd final : public Canvas {
  private: HWND hwnd_;
  private: std::unique_ptr<SwapChain> swap_chain_;

  public: CanvasForHwnd(HWND hwnd);
  public: virtual ~CanvasForHwnd();

  // Canvas
  private: virtual void AddDirtyRectImpl(const RectF& new_dirty_rect) override;
  private: virtual void DidCallEndDraw() override;
  private: virtual void DidChangeBounds(const RectF& new_bounds) override;
  private: virtual void DidLostRenderTarget() override;
  private: virtual ID2D1RenderTarget* GetRenderTarget() const override;

  DISALLOW_COPY_AND_ASSIGN(CanvasForHwnd);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_canvas_h)
