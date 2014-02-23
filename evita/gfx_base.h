// Graphics
//
// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
#if !defined(INCLUDE_evita_gfx_base_h)
#define INCLUDE_evita_gfx_base_h

#include <memory>

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "common/memory/singleton.h"
#include "common/win/rect.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/rect_f.h"


#pragma warning(push)
// warning C4263: 'function' : member function does not override any base
// class virtual member function
#pragma warning(disable: 4263)
// warning C4264: 'virtual_function' : no override available for virtual
// member function from base 'class'; function is hidden
#pragma warning(disable: 4264)

#include <dwrite.h>
#pragma warning(pop)

//'warning C4625: derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(push)
#pragma warning(disable: 4625)
#include <wincodec.h>
#pragma warning(pop)

namespace gfx {

typedef common::win::Point Point;
typedef common::win::Rect Rect;
typedef common::win::Size Size;

class Object {
  protected: Object() {}
  protected: ~Object() {}
  DISALLOW_COPY_AND_ASSIGN(Object);
};

template<class T>
class SimpleObject_ : public Object {
  private: const common::ComPtr<T> ptr_;
  protected: SimpleObject_(T* ptr) : ptr_(ptr) {}
  protected: SimpleObject_(common::ComPtr<T>&& ptr) : ptr_(std::move(ptr)) {}
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

class FactorySet : public base::RefCounted<FactorySet>,
                   public common::ComInit,
                   public common::Singleton<FactorySet>,
                   public DpiHandler,
                   public Object {
  friend class common::Singleton<FactorySet>;
  private: common::ComPtr<ID2D1Factory> d2d1_factory_;
  private: common::ComPtr<IDWriteFactory> dwrite_factory_;
  private: common::ComPtr<IWICImagingFactory> image_factory_;

  private: FactorySet();
  public: ~FactorySet() = default;
  public: static ID2D1Factory& d2d1() {
    return *instance()->d2d1_factory_;
  }
  public: static IDWriteFactory& dwrite() {
    return *instance()->dwrite_factory_;
  }
  public: static IWICImagingFactory& image() {
    return *instance()->image_factory_;
  }

  public: static SizeF AlignToPixel(const SizeF& size) {
    return instance()->DpiHandler::AlignToPixel(size);
  }
  public: static SizeF CeilToPixel(const SizeF& size) {
    return instance()->DpiHandler::CeilToPixel(size);
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
  private: const scoped_refptr<FactorySet> factory_set_;
  public: TextFormat(const base::string16& font_face_name, float font_size);
  public: TextFormat(const LOGFONT& log_font);
  public: std::unique_ptr<TextLayout> CreateLayout(const char16*, int) const;
  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

class TextLayout : public SimpleObject_<IDWriteTextLayout> {
  public: TextLayout(const LOGFONT& log_font);
  public: SIZE GetMetrics() const;
  DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

//////////////////////////////////////////////////////////////////////
//
// Graphics
//
class Graphics : public Object, public DpiHandler {
  public: class Observer {
    public: Observer();
    public: virtual ~Observer();

    public: virtual void ShouldDiscardResources() = 0;
  };

  private: mutable int batch_nesting_level_;
  private: scoped_refptr<FactorySet> factory_set_;
  private: HWND hwnd_;
  private: ObserverList<Observer> observers_;
  private: common::ComPtr<ID2D1RenderTarget> render_target_;
  private: mutable void* work_;

  public: class AxisAlignedClipScope {
    private: const Graphics& gfx_;
    public: AxisAlignedClipScope(const Graphics& gfx, const RectF& rect);
    public: ~AxisAlignedClipScope();
    DISALLOW_COPY_AND_ASSIGN(AxisAlignedClipScope);
  };

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

  private: Graphics(ID2D1RenderTarget* render_target);
  public: Graphics(Graphics&& other);
  public: Graphics();
  public: ~Graphics();

  public: operator bool() const {
    return render_target_;
  }
  public: operator ID2D1RenderTarget*() const {
    return render_target_.get();
  }
  public: ID2D1RenderTarget* operator->() const {
    return render_target_.get();
  }

  public: Graphics& operator=(Graphics&& other);

  // |drawing()| is for debugging.
  public: bool drawing() const { return batch_nesting_level_; }
  public: const FactorySet& factory_set() const { return *factory_set_; }

  public: ID2D1RenderTarget& render_target() const {
    DCHECK(render_target_) << "No ID2D1RenderTarget";
    return *render_target_.get();
  }

  public: template<typename T> T* work() const { 
    return reinterpret_cast<T*>(work_); 
  }
  public: void set_work(void* ptr) const { work_ = ptr; }

  // [A]
  public: void AddObserver(Observer* observer);

  // [B]
  public: void BeginDraw() const;

  // [C]
  public: Graphics CreateCompatible(const gfx::SizeF& size) const;

  // [D]
  public: void DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity = 1.0f,
                          D2D1_BITMAP_INTERPOLATION_MODE mode =
                              D2D1_BITMAP_INTERPOLATION_MODE_LINEAR) const;
  public: void DrawLine(const Brush& brush, int sx, int sy, int ex, int ey,
                        float strokeWidth = 1) const {
    DCHECK(drawing());
    render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                             strokeWidth);
  }

  public: void DrawLine(const Brush& brush,
                        float sx, float sy,
                        float ex, float ey,
                        float strokeWidth = 1) const {
    DCHECK(drawing());
    render_target().DrawLine(PointF(sx, sy), PointF(ex, ey), brush,
                             strokeWidth);
  }

  public: void DrawRectangle(const Brush& brush, const RECT& rc,
                             float strokeWidth = 1) const {
    DrawRectangle(brush, RectF(rc), strokeWidth);
  }

  public: void DrawRectangle(const Brush& brush, const RectF& rect,
                             float strokeWidth = 1) const {
    DCHECK(drawing());
    DCHECK(rect);
    render_target().DrawRectangle(rect, brush, strokeWidth);
  }

  public: void DrawText(const TextFormat& text_format,
                        const Brush& brush,
                        const RECT& rc,
                        const char16* pwch, size_t cwch) const {
    DCHECK(drawing());
    auto rect = RectF(rc);
    DCHECK(rect);
    render_target().DrawText(pwch, static_cast<uint32_t>(cwch), text_format,
                             rect, brush);
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
    DCHECK(drawing());
    DCHECK(rect);
    render_target().FillRectangle(rect, brush);
  }

  public: void Flush() const;

  // [I]
  public: void Init(HWND hwnd);

  // [R]
  private: void Reinitialize();
  public: void RemoveObserver(Observer* observer);
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

#endif //!defined(INCLUDE_evita_gfx_base_h)
