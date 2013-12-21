// Graphics
//
// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
#if !defined(INCLUDE_gfx_base_h)
#define INCLUDE_gfx_base_h

#include "base/win/scoped_comptr.h"
#include "gfx/rect.h"
#include "gfx/rect_f.h"
#include "./li_util.h"

// warning C4263: 'function' : member function does not override any base
// class virtual member function
#pragma warning(disable: 4263)
// warning C4264: 'virtual_function' : no override available for virtual
// member function from base 'class'; function is hidden
#pragma warning(disable: 4264)

#include <dwrite.h>
#pragma warning(default: 4263)
#pragma warning(default: 4264)

//'warning C4625: derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4625)
#include <wincodec.h>
#pragma warning(default: 4625)

namespace gfx {

class Object {
  protected: Object() {}
  protected: ~Object() {}
  DISALLOW_COPY_AND_ASSIGN(Object);
};

template<class T>
class SimpleObject_ : public Object {
  private: const base::ComPtr<T> ptr_;
  protected: SimpleObject_(T* ptr) : ptr_(ptr) {}
  protected: SimpleObject_(base::ComPtr<T>&& ptr) : ptr_(std::move(ptr)) {}
  public: operator T*() const { return ptr_; }
  public: T* operator->() const { return ptr_; }
  DISALLOW_COPY_AND_ASSIGN(SimpleObject_);
};

//////////////////////////////////////////////////////////////////////
//
// Graphics objects
//
typedef D2D1::ColorF ColorF;
class Bitmap;
class Brush;
class FactorySet;
class FontFace;
class Graphics;
class TextFormat;
class TextLayout;

class Bitmap : public SimpleObject_<ID2D1Bitmap> {
  public: Bitmap(const Graphics& gfx, HICON hIcon);
  public: Bitmap(const Graphics& gfx, SizeU size);
  public: explicit Bitmap(const Graphics& gfx);
  DISALLOW_COPY_AND_ASSIGN(Bitmap);
};

class Brush : public SimpleObject_<ID2D1SolidColorBrush> {
  public: Brush(const Graphics& gfx, ColorF color);
  #if _DEBUG
    public: ~Brush();
  #endif
  DISALLOW_COPY_AND_ASSIGN(Brush);
};

class DpiHandler {
  private: SizeF dpi_;
  private: SizeF pixels_per_dip_;
  public: const SizeF& pixels_per_dip() const { return pixels_per_dip_; }
  public: SizeF AlignToPixel(const SizeF& size) const;
  public: SizeF CeilToPixel(const SizeF& size) const;
  public: SizeF FloorToPixel(const SizeF& size) const;
  protected: void UpdateDpi(const SizeF&);
};

class FactorySet : public RefCounted_<FactorySet>,
                   public base::ComInit,
                   public DpiHandler,
                   public Object {
  private: base::ComPtr<ID2D1Factory> d2d1_factory_;
  private: base::ComPtr<IDWriteFactory> dwrite_factory_;
  private: base::ComPtr<IWICImagingFactory> image_factory_;

  public: FactorySet();
  public: ~FactorySet() {}
  public: static FactorySet& instance();
  public: static ID2D1Factory& d2d1() {
    return *instance().d2d1_factory_;
  }
  public: static IDWriteFactory& dwrite() {
    return *instance().dwrite_factory_;
  }
  public: static IWICImagingFactory& image() {
    return *instance().image_factory_;
  }

  public: static SizeF AlignToPixel(const SizeF& size) {
    return instance().DpiHandler::AlignToPixel(size);
  }
  public: static SizeF CeilToPixel(const SizeF& size) {
    return instance().DpiHandler::CeilToPixel(size);
  }
  DISALLOW_COPY_AND_ASSIGN(FactorySet);
};

class FontFace : public SimpleObject_<IDWriteFontFace> {
  private: const DWRITE_FONT_METRICS metrics_;
  public: FontFace(const char16* family_name);
  public: const DWRITE_FONT_METRICS& metrics() const { return metrics_; }
  DISALLOW_COPY_AND_ASSIGN(FontFace);
};

class TextFormat : public SimpleObject_<IDWriteTextFormat> {
  private: const ScopedRefCount_<FactorySet> factory_set_;
  public: TextFormat(const LOGFONT& log_font);
  public: base::OwnPtr<TextLayout> CreateLayout(const char16*, int) const;
  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

class TextLayout : public SimpleObject_<IDWriteTextLayout> {
  public: TextLayout(const LOGFONT& log_font);
  public: SIZE GetMetrics() const;
  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

class Graphics : public Object, public DpiHandler {
  private: mutable int batch_nesting_level_;
  private: ScopedRefCount_<FactorySet> factory_set_;
  private: HWND hwnd_;
  private: ID2D1HwndRenderTarget* render_target_;
  private: mutable void* work_;

  public: class DrawingScope {
    private: const Graphics& gfx_;
    public: DrawingScope(const Graphics& gfx) : gfx_(gfx) {
      gfx_.BeginDraw();
    }
    public: ~DrawingScope() {
      // TODO: Should DrawingScope take mutable Graphics?
      const_cast<Graphics&>(gfx_).EndDraw();
    }
    DISALLOW_COPY_AND_ASSIGN(DrawingScope);
  };
  friend class DrawingScope;

  public: Graphics();
  public: ~Graphics();

  public: operator ID2D1HwndRenderTarget*() const {
    return &render_target();
  }
  public: ID2D1HwndRenderTarget* operator->() const {
    return &render_target();
  }

  // |drawing()| is for debugging.
  public: bool drawing() const { return batch_nesting_level_; }
  public: const FactorySet& factory_set() const { return *factory_set_; }

  public: ID2D1HwndRenderTarget& render_target() const {
    ASSERT(!!render_target_);
    return *render_target_;
  }

  public: template<typename T> T* work() const { 
    return reinterpret_cast<T*>(work_); 
  }
  public: void set_work(void* ptr) const { work_ = ptr; }

  // [B]
  public: void BeginDraw() const;

  // [D]
  public: void DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth = 1) const {
    ASSERT(drawing());
    render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                             strokeWidth);
  }

  public: void DrawLine(const Brush& brush,
                        float sx, float sy,
                        float ex, float ey,
                        float strokeWidth = 1) const {
    ASSERT(drawing());
    render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                             strokeWidth);
  }

  public: void DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth = 1) const {
    DrawRectangle(brush, RectF(rc), strokeWidth);
  }

  public: void DrawRectangle(const Brush& brush, const RectF& rect,
                             float strokeWidth = 1) const {
    ASSERT(drawing());
    ASSERT(!!rect);
    render_target().DrawRectangle(rect, brush, strokeWidth);
  }

  public: void DrawText(const TextFormat& text_format,
                        const Brush& brush,
                        const RECT& rc,
                        const char16* pwch, size_t cwch) const {
    ASSERT(drawing());
    auto rect = RectF(rc);
    ASSERT(!!rect);
    render_target().DrawText(pwch, cwch, text_format, rect, brush);
  }

  // [E]
  // Returns true if succeeded.
  public: bool EndDraw();

  // [F]
  public: void FillRectangle(const Brush& brush, int left, int top,
                             int right, int bottom) const {
    render_target().FillRectangle(RectF(left, top, right, bottom), brush);
  }

  public: void FillRectangle(const Brush& brush, float left, float top,
                             float right, float bottom) const {
    FillRectangle(brush, RectF(left, top, right, bottom));
  }

  public: void FillRectangle(const Brush& brush, const RECT& rc) const {
    FillRectangle(brush, RectF(rc));
  }

  public: void FillRectangle(const Brush& brush, const RectF& rect) const {
    ASSERT(drawing());
    ASSERT(!!rect);
    render_target().FillRectangle(rect, brush);
  }

  public: void Flush() const;

  // [I]
  public: void Init(HWND hwnd);

  // [R]
  private: void Reinitialize();
  public: void Resize(const Rect& rc) const;

  DISALLOW_COPY_AND_ASSIGN(Graphics);
};

// Helper functions
inline ColorF blackColor() {
  return ColorF(ColorF::Black);
}

inline ColorF grayColor() {
  return ColorF(ColorF::LightGray);
}

inline ColorF sysColor(int name, float alpha = 1) {
  auto const colorRef = ::GetSysColor(name);
  return ColorF(static_cast<float>(GetRValue(colorRef)) / 255,
                static_cast<float>(GetGValue(colorRef)) / 255,
                static_cast<float>(GetBValue(colorRef)) / 255,
                alpha);
}

inline ColorF whiteColor() {
  return ColorF(ColorF::White);
}

} // namespace gfx

#endif //!defined(INCLUDE_gfx_base_h)
