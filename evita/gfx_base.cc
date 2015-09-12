//////////////////////////////////////////////////////////////////////////////
//
// Graphics
//
// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
#include <math.h>
#include <utility>

#include "evita/gfx_base.h"
#include "evita/gfx/text_layout.h"
#include "evita/precomp.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace gfx {

namespace {

ID2D1Factory1& CreateD2D1Factory() {
  ID2D1Factory1* factory;
  D2D1_FACTORY_OPTIONS options;
#if _DEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
  options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
  COM_VERIFY(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options,
                                 &factory));
  return *factory;
}

IDWriteFactory& CreateDWriteFactory() {
  IDWriteFactory* factory;
  COM_VERIFY(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                   __uuidof(IDWriteFactory),
                                   reinterpret_cast<IUnknown**>(&factory)));
  return *factory;
}

IWICImagingFactory& CreateImageFactory() {
  IWICImagingFactory* factory;
  COM_VERIFY(::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
  return *factory;
}

common::ComPtr<ID2D1SolidColorBrush> CreateSolidColorBrush(Canvas* canvas,
                                                           ColorF color) {
  common::ComPtr<ID2D1SolidColorBrush> brush;
  COM_VERIFY((*canvas)->CreateSolidColorBrush(color, &brush));
  return brush;
}

float MultipleOf(float x, float unit) {
  return ::ceilf(x / unit) * unit;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Brush
//
Brush::Brush(Canvas* canvas, ColorF color)
    : SimpleObject_(CreateSolidColorBrush(canvas, color)) {}

Brush::Brush(Canvas* canvas, D2D1::ColorF::Enum name)
    : Brush(canvas, ColorF(name)) {}

Brush::Brush(Canvas* canvas, float red, float green, float blue, float alpha)
    : SimpleObject_(
          CreateSolidColorBrush(canvas, ColorF(red, green, blue, alpha))) {}

Brush::~Brush() {
#if _DEBUG
  auto const ref_count = (*this)->AddRef();
  DCHECK_EQ(2u, ref_count);
  (*this)->Release();
#endif
}

//////////////////////////////////////////////////////////////////////
//
// DpiHandler
//
SizeF DpiHandler::AlignToPixel(const SizeF& size) const {
  DCHECK(!pixels_per_dip_.empty());
  return SizeF(MultipleOf(size.width, pixels_per_dip_.width),
               MultipleOf(size.height, pixels_per_dip_.height));
}

SizeF DpiHandler::CeilToPixel(const SizeF& size) const {
  return SizeF(::ceilf(size.width * pixels_per_dip_.width),
               ::ceilf(size.height * pixels_per_dip_.height));
}

SizeF DpiHandler::FloorToPixel(const SizeF& size) const {
  return SizeF(::floorf(size.width * pixels_per_dip_.width),
               ::floorf(size.height * pixels_per_dip_.height));
}

void DpiHandler::UpdateDpi(const SizeF& dpi) {
  dpi_ = dpi;
  // Note: All render targets except for ID2D1HwndRenderTarget, DPI values
  // are 96 DPI.
  float const default_dpi = 96.0f;
  pixels_per_dip_ = dpi_ / default_dpi;
  pixels_per_dip_ = SizeF(96.0f / dpi_.width, 96.0f / dpi_.height);
}

//////////////////////////////////////////////////////////////////////
//
// FactorySet
//
FactorySet::FactorySet()
    : d2d1_factory_(CreateD2D1Factory()),
      dwrite_factory_(CreateDWriteFactory()),
      image_factory_(CreateImageFactory()) {
  SizeF dpi;
  d2d1_factory_->GetDesktopDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
}

}  // namespace gfx
