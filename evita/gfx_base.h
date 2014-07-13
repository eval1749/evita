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
#include "evita/gfx/color_f.h"
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

//'warning C4625: derived class' : copy constructor could not be generated
// because a base class copy constructor is inaccessible
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
class ColorF;
class Bitmap;
class Brush;
class FactorySet;
class FontFace;
class Canvas;
class TextFormat;
class TextLayout;

class Brush : public SimpleObject_<ID2D1SolidColorBrush> {
  public: Brush(Canvas* canvas, ColorF color);
  public: Brush(Canvas* canvas, float red, float green, float blue,
                float alpha = 1.0f);
  public: ~Brush();
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
  private: common::ComPtr<ID2D1Factory1> d2d1_factory_;
  private: common::ComPtr<IDWriteFactory> dwrite_factory_;
  private: common::ComPtr<IWICImagingFactory> image_factory_;

  private: FactorySet();
  public: ~FactorySet() = default;
  public: static ID2D1Factory1& d2d1() {
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

#include "evita/gfx/canvas.h"

#endif //!defined(INCLUDE_evita_gfx_base_h)
