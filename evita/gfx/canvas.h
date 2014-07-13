// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_canvas_h)
#define INCLUDE_evita_gfx_canvas_h

#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"

interface IDXGISwapChain1;

namespace gfx {

class Bitmap;

//////////////////////////////////////////////////////////////////////
//
// Canvas
//
class Canvas final : public Object, public DpiHandler {
  public: enum class DwmSupport {
    NotSupportDwm,
    SupportDwm,
  };

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

  private: mutable int batch_nesting_level_;
  private: mutable std::vector<Rect> dirty_rects_;
  private: mutable common::ComPtr<IDXGISwapChain1> dxgi_swap_chain_;
  private: const DwmSupport dwm_support_;
  private: scoped_refptr<FactorySet> factory_set_;
  private: HWND hwnd_;
  private: ObserverList<Observer> observers_;
  private: mutable std::unique_ptr<Bitmap> screen_bitmap_;
  private: mutable Rect target_bounds_;
  private: mutable common::ComPtr<ID2D1RenderTarget> render_target_;
  private: mutable void* work_;

  public: explicit Canvas(DwmSupport dwm_support);
  public: Canvas();
  public: ~Canvas();

  public: explicit operator bool() const {
    return render_target_;
  }
  public: operator ID2D1RenderTarget*() const {
    return render_target_.get();
  }
  public: ID2D1RenderTarget* operator->() const {
    return render_target_.get();
  }

  public: void set_dirty_rect(const Rect& rect) const;
  public: void set_dirty_rect(const RectF& rect) const;
  // |drawing()| is for debugging.
  public: bool drawing() const { return batch_nesting_level_; }
  public: const FactorySet& factory_set() const { return *factory_set_; }
  public: ID2D1RenderTarget& render_target() const;
  public: Bitmap* screen_bitmap() const { return screen_bitmap_.get(); }
  public: template<typename T> T* work() const { 
    return reinterpret_cast<T*>(work_); 
  }
  public: void set_work(void* ptr) const { work_ = ptr; }

  // [A]
  public: void AddObserver(Observer* observer);

  // [B]
  public: void BeginDraw();

  // [D]
  public: void DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity = 1.0f,
                          D2D1_BITMAP_INTERPOLATION_MODE mode =
                              D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
  public: void DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth = 1);
  public: void DrawLine(const Brush& brush,
                        float sx, float sy,
                        float ex, float ey,
                        float strokeWidth = 1);
  public: void DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth = 1);
  public: void DrawRectangle(const Brush& brush, const RectF& rect,
                             float strokeWidth = 1);
  public: void DrawText(const TextFormat& text_format,
                        const Brush& brush,
                        const RECT& rc,
                        const base::char16* pwch, size_t cwch);

  // [E]
  // Returns true if succeeded.
  public: bool EndDraw();

  // [F]
  public: void FillRectangle(const Brush& brush, int left, int top,

                             int right, int bottom);
  public: void FillRectangle(const Brush& brush, float left, float top,
                             float right, float bottom);
  public: void FillRectangle(const Brush& brush, const RECT& rc);
  public: void FillRectangle(const Brush& brush, const RectF& rect);
  public: void Flush();

  // [I]
  public: void Init(HWND hwnd);

  // [R]
  private: void Reinitialize();
  public: void RemoveObserver(Observer* observer);
  public: void Resize(const Rect& bounds);

  // [S]
  public: bool Canvas::SaveScreenImage(const RectF& bounds);
  private: void SetupRenderTarget(ID2D1DeviceContext* d2d_device_context);

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_canvas_h)
