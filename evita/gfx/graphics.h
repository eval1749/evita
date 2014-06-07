// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_graphics_h)
#define INCLUDE_evita_gfx_graphics_h

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
class Canvas : public Object, public DpiHandler {
  public: class Observer {
    public: Observer();
    public: virtual ~Observer();

    public: virtual void ShouldDiscardResources() = 0;
  };

  public: class AxisAlignedClipScope {
    private: const Canvas& gfx_;
    public: AxisAlignedClipScope(const Canvas& gfx, const RectF& rect);
    public: ~AxisAlignedClipScope();
    DISALLOW_COPY_AND_ASSIGN(AxisAlignedClipScope);
  };

  public: class DrawingScope {
    private: const Canvas& gfx_;
    public: DrawingScope(const Canvas& gfx);
    public: ~DrawingScope();
    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };
  friend class DrawingScope;

  private: mutable int batch_nesting_level_;
  private: mutable std::vector<Rect> dirty_rects_;
  private: mutable common::ComPtr<IDXGISwapChain1> dxgi_swap_chain_;
  private: scoped_refptr<FactorySet> factory_set_;
  private: HWND hwnd_;
  private: ObserverList<Observer> observers_;
  private: mutable std::unique_ptr<Bitmap> screen_bitmap_;
  private: mutable Rect target_bounds_;
  private: mutable common::ComPtr<ID2D1RenderTarget> render_target_;
  private: mutable void* work_;

  private: Canvas(ID2D1RenderTarget* render_target);
  public: Canvas(Canvas&& other);
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

  public: Canvas& operator=(Canvas&& other);

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
  public: void BeginDraw() const;

  // [C]
  public: Canvas CreateCompatible(const gfx::SizeF& size) const;

  // [D]
  public: void DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity = 1.0f,
                          D2D1_BITMAP_INTERPOLATION_MODE mode =
                              D2D1_BITMAP_INTERPOLATION_MODE_LINEAR) const;
  public: void DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth = 1) const;
  public: void DrawLine(const Brush& brush,
                        float sx, float sy,
                        float ex, float ey,
                        float strokeWidth = 1) const;
  public: void DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth = 1) const;
  public: void DrawRectangle(const Brush& brush, const RectF& rect,
                             float strokeWidth = 1) const;
  public: void DrawText(const TextFormat& text_format,
                        const Brush& brush,
                        const RECT& rc,
                        const base::char16* pwch, size_t cwch) const;

  // [E]
  // Returns true if succeeded.
  public: bool EndDraw();

  // [F]
  public: void FillRectangle(const Brush& brush, int left, int top,

                             int right, int bottom) const;
  public: void FillRectangle(const Brush& brush, float left, float top,
                             float right, float bottom) const;
  public: void FillRectangle(const Brush& brush, const RECT& rc) const;
  public: void FillRectangle(const Brush& brush, const RectF& rect) const;
  public: void Flush() const;

  // [I]
  public: void Init(HWND hwnd);

  // [R]
  private: void Reinitialize();
  public: void RemoveObserver(Observer* observer);
  public: void Resize(const Rect& rc) const;

  // [S]
  public: bool Canvas::SaveScreenImage(const RectF& rect) const;

  DISALLOW_COPY_AND_ASSIGN(Canvas);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_graphics_h)
