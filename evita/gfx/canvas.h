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
class SwapChain;

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
class Canvas : public Object, public DpiHandler {
  public: class Observer {
    public: Observer();
    public: virtual ~Observer();

    public: virtual void ShouldDiscardResources() = 0;
  };

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
    private: Canvas* const canvas_;
    private: gfx::PointF offset_;
    private: D2D1::Matrix3x2F transform_;

    public: ScopedState(Canvas* canvas);
    public: ~ScopedState();

    DISALLOW_COPY_AND_ASSIGN(ScopedState);
  };
  friend class ScopedState;

  private: int batch_nesting_level_;
  private: RectF bounds_;
  private: scoped_refptr<FactorySet> factory_set_;
  private: ObserverList<Observer> observers_;
  private: gfx::PointF offset_;
  private: std::unique_ptr<Bitmap> screen_bitmap_;
  private: void* work_;

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

  public: const gfx::RectF bounds() const { return bounds_; }
  // |drawing()| is for debugging.
  public: bool drawing() const { return batch_nesting_level_; }
  public: const FactorySet& factory_set() const { return *factory_set_; }
  public: Bitmap* screen_bitmap() const { return screen_bitmap_.get(); }
  public: template<typename T> T* work() const { 
    return reinterpret_cast<T*>(work_); 
  }
  public: void set_work(void* ptr) { work_ = ptr; }

  // [A]
  public: virtual void AddDirtyRect(const RectF& new_dirty_rect);
  protected: virtual void AddDirtyRectImpl(const RectF& new_dirty_rect);
  public: void AddObserver(Observer* observer);

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
  public: virtual ID2D1RenderTarget* GetRenderTarget() const = 0;

  // [R]
  public: void RemoveObserver(Observer* observer);

  // [S]
  public: bool Canvas::SaveScreenImage(const RectF& bounds);
  public: void SetBounds(const RectF& bounds);
  protected: void SetInitialBounds(const RectF& bounds);

  // Set canvas origin to |new_origin| in current coordinate. This function is
  // used for setting canvas coordinate for child window.
  // See |ui::Widget::OnDraw()| for example usage.
  public: void SetOrigin(const gfx::PointF new_origin);

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

//////////////////////////////////////////////////////////////////////
//
// CanvasForHwnd
//
class CanvasForHwnd : public Canvas {
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

//////////////////////////////////////////////////////////////////////
//
// LegacyCanvasForHwnd
//
class LegacyCanvasForHwnd : public Canvas {
  private: HWND hwnd_;
  private: common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target_;

  public: LegacyCanvasForHwnd(HWND hwnd);
  public: virtual ~LegacyCanvasForHwnd();

  private: void AttachRenderTarget();

  // Canvas
  private: virtual void DidChangeBounds(const RectF& new_bounds) override;
  private: virtual void DidLostRenderTarget() override;
  private: virtual ID2D1RenderTarget* GetRenderTarget() const override;

  DISALLOW_COPY_AND_ASSIGN(LegacyCanvasForHwnd);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_canvas_h)
