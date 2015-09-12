// Graphics
//
// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
#ifndef EVITA_GFX_BASE_H_
#define EVITA_GFX_BASE_H_

#pragma warning(push)
// warning C4263: 'function' : member function does not override any base
// class virtual member function
#pragma warning(disable : 4263)
// warning C4264: 'virtual_function' : no override available for virtual
// member function from base 'class'; function is hidden
#pragma warning(disable : 4264)

#include <dwrite.h>
#pragma warning(pop)

// warning C4625: derived class' : copy constructor could not be generated
// because a base class copy constructor is inaccessible
#pragma warning(push)
#pragma warning(disable : 4625)
#include <wincodec.h>
#pragma warning(pop)

#include <memory>

#include "base/basictypes.h"
#include "base/observer_list.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "common/memory/singleton.h"
#include "common/win/rect.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_f.h"

namespace gfx {

typedef common::win::Point Point;
typedef common::win::Rect Rect;
typedef common::win::Size Size;

class Object {
 protected:
  Object() {}
  ~Object() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(Object);
};

template <class T>
class SimpleObject_ : public Object {
 public:
  operator T*() const { return ptr_; }
  T* operator->() const { return ptr_; }

 protected:
  explicit SimpleObject_(T* ptr) : ptr_(ptr) {}
  explicit SimpleObject_(common::ComPtr<T>&& ptr) : ptr_(std::move(ptr)) {}

 private:
  const common::ComPtr<T> ptr_;

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

class Brush final : public SimpleObject_<ID2D1SolidColorBrush> {
 public:
  Brush(Canvas* canvas, ColorF color);
  Brush(Canvas* canvas, D2D1::ColorF::Enum name);
  Brush(Canvas* canvas, float red, float green, float blue, float alpha = 1.0f);
  ~Brush();

 private:
  DISALLOW_COPY_AND_ASSIGN(Brush);
};

class DpiHandler {
 public:
  const SizeF& pixels_per_dip() const { return pixels_per_dip_; }
  SizeF AlignToPixel(const SizeF& size) const;
  SizeF CeilToPixel(const SizeF& size) const;
  SizeF FloorToPixel(const SizeF& size) const;

 protected:
  void UpdateDpi(const SizeF&);

 private:
  SizeF dpi_;
  SizeF pixels_per_dip_;
};

class FactorySet final : public base::RefCounted<FactorySet>,
                         public common::ComInit,
                         public common::Singleton<FactorySet>,
                         public DpiHandler,
                         public Object {
 public:
  ~FactorySet() = default;
  static ID2D1Factory1& d2d1() { return *instance()->d2d1_factory_; }
  static IDWriteFactory& dwrite() { return *instance()->dwrite_factory_; }
  static IWICImagingFactory& image() { return *instance()->image_factory_; }

  static SizeF AlignToPixel(const SizeF& size) {
    return instance()->DpiHandler::AlignToPixel(size);
  }
  static SizeF CeilToPixel(const SizeF& size) {
    return instance()->DpiHandler::CeilToPixel(size);
  }

 private:
  friend class common::Singleton<FactorySet>;

  common::ComPtr<ID2D1Factory1> d2d1_factory_;
  common::ComPtr<IDWriteFactory> dwrite_factory_;
  common::ComPtr<IWICImagingFactory> image_factory_;

  FactorySet();
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
                static_cast<float>(GetBValue(colorRef)) / 255, alpha);
}

inline ColorF whiteColor() {
  return ColorF(ColorF::White);
}

}  // namespace gfx

#include "evita/gfx/canvas.h"

#endif  // EVITA_GFX_BASE_H_
