#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Graphics
//
// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
#include "gfx_base.h"

#include <math.h>
#include <utility>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

#define DEBUG_DRAW 0

std::ostream& operator<<(std::ostream& ostream,
                         D2D1_TEXT_ANTIALIAS_MODE mode) {
  switch (mode) {
    case D2D1_TEXT_ANTIALIAS_MODE_DEFAULT:
      return ostream << "DEFAULT";
    case D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE:
      return ostream << "CLEARTYPE";
    case D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE:
      return ostream << "GRAYSCALE";
    case D2D1_TEXT_ANTIALIAS_MODE_ALIASED:
      return ostream << "ALIASED";
  }
  return ostream << "UNKNOWN_" << static_cast<int>(mode);
}

namespace gfx {

namespace {

#if 0
class NullFont : IDWriteFont {
  public: STDMETHOD_(ULONG, AddRef)() override { return 1; }
  public: STDMETHOD_(ULONG, Release)() override { return 1; }
  public: STDMETHOD(QueryInterface)(REFIID, void**) override { 
    return E_NOTIMPL;
  }

  //IDWriteFont
  public: STDMETHOD(CreateFontFace)(IDWriteFontFace**) override {
    return E_NOTIMPL;
  }
  public: STDMETHOD(GetFaceNames)(IDWriteLocalizedStrings**) override {
    return E_NOTIMPL;
  }
  public: STDMETHOD(GetFontFamily)(IDWriteFontFamily**) override {
    return E_NOTIMPL;
  }
  public: STDMETHOD(GetInformationalStrings)(DWRITE_INFORMATIONAL_STRING_ID,
                                             IDWriteLocalizedStrings**,
                                             BOOL*) override {
    return E_NOTIMPL;
  }
  public: STDMETHOD_(void, GetMetrics)(DWRITE_FONT_METRICS* metrics) override {
    ::ZeroMemory(&metrics, sizeof(metrics));
  }
  public: STDMETHOD_(DWRITE_FONT_SIMULATIONS, GetSimulations)() override {
    return DWRITE_FONT_SIMULATIONS_NONE;
  }
  public: STDMETHOD_(DWRITE_FONT_STRETCH, GetStretch)() override {
    return DWRITE_FONT_STRETCH_NORMAL;
  }
  public: STDMETHOD_(DWRITE_FONT_STYLE, GetStyle)() override {
    return DWRITE_FONT_STYLE_NORMAL;
  }
  public: STDMETHOD_(DWRITE_FONT_WEIGHT, GetWeight)() override {
    return DWRITE_FONT_WEIGHT_NORMAL;
  }
  public: STDMETHOD(HasCharacter)(UINT32, BOOL*) override {
    return E_NOTIMPL;
  }
  public: STDMETHOD_(BOOL, IsSymbolFont)() override {
    return false;
  }
};
#endif

common::ComPtr<ID2D1Bitmap> CreateBitmap(const Graphics& gfx, HICON hIcon) {
  common::ComPtr<IWICBitmap> icon;
  COM_VERIFY(gfx::FactorySet::image().CreateBitmapFromHICON(
      hIcon, &icon));
 common::ComPtr<IWICFormatConverter> converter;
 COM_VERIFY(gfx::FactorySet::image().
    CreateFormatConverter(&converter));
 COM_VERIFY(converter->Initialize(
      icon,
      GUID_WICPixelFormat32bppPBGRA,
      WICBitmapDitherTypeNone,
      nullptr,
      0,
      WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY(gfx->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::move(bitmap);
}

ID2D1Factory& CreateD2D1Factory() {
  ID2D1Factory* factory;
  COM_VERIFY(::D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED,
      &factory));
  return *factory;
}

IDWriteFactory& CreateDWriteFactory() {
  IDWriteFactory* factory;
  COM_VERIFY(::DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory),
      reinterpret_cast<IUnknown**>(&factory)));
  return *factory;
}

common::ComPtr<IDWriteFontFace> CreateFontFace(const char16* family_name) {
  common::ComPtr<IDWriteFontCollection> font_collection;
  COM_VERIFY(gfx::FactorySet::dwrite().
      GetSystemFontCollection(&font_collection, false));

  uint32 index;
  BOOL exists;
  COM_VERIFY(font_collection->FindFamilyName(family_name, &index, &exists));
  if (!exists)
   return CreateFontFace(L"Courier New");

  common::ComPtr<IDWriteFontFamily> font_family;
  COM_VERIFY(font_collection->GetFontFamily(index, &font_family));

  common::ComPtr<IDWriteFont> font;
  COM_VERIFY(font_family->GetFirstMatchingFont(
    DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    DWRITE_FONT_STYLE_NORMAL, // normal, italic or oblique
    &font));

  common::ComPtr<IDWriteFontFace> font_face;
  COM_VERIFY(font->CreateFontFace(&font_face));
  return std::move(font_face);
}

IWICImagingFactory& CreateImageFactory() {
  IWICImagingFactory* factory;
  COM_VERIFY(::CoCreateInstance(
      CLSID_WICImagingFactory,
      nullptr,
      CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(&factory)));
  return *factory;
}

common::ComPtr<ID2D1SolidColorBrush> CreateSolidColorBrush(const Graphics& gfx,
                                                         ColorF color) {
  common::ComPtr<ID2D1SolidColorBrush> brush;
  COM_VERIFY(gfx->CreateSolidColorBrush(color, &brush));
  return brush;
}

static float pt2dip(int pt) {
  return static_cast<float>(pt) * 96.0f / 72.0f;
}

common::ComPtr<IDWriteTextFormat> CreateTextFormat(
    const base::string16 font_face_name, float font_size) {
  auto size = FactorySet::CeilToPixel(SizeF(0.0f, font_size));
  common::ComPtr<IDWriteTextFormat> text_format;
  COM_VERIFY(FactorySet::instance()->dwrite().CreateTextFormat(
    font_face_name.c_str(),
    nullptr,
    DWRITE_FONT_WEIGHT_REGULAR,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    size.height,
    L"en-us",
    &text_format));
  return std::move(text_format);
}

DWRITE_FONT_METRICS GetFontMetrics(IDWriteFontFace* font) {
  DWRITE_FONT_METRICS metrics;
  font->GetMetrics(&metrics);
  return metrics;
}

float MultipleOf(float x, float unit) {
  return ::ceilf(x / unit) * unit;
}

} // namespace

//////////////////////////////////////////////////////////////////////
//
// Bitmap
//
namespace {
common::ComPtr<ID2D1Bitmap> CreateBitmap(const Graphics& gfx, SizeU size) {
  common::ComPtr<ID2D1Bitmap> bitmap;
  D2D1_BITMAP_PROPERTIES props;
  props.pixelFormat = gfx->GetPixelFormat();
  gfx->GetDpi(&props.dpiX, &props.dpiY);
  COM_VERIFY(gfx->CreateBitmap(size, props, &bitmap));
  return std::move(bitmap);
}
}

Bitmap::Bitmap(const Graphics& gfx, HICON hIcon)
    : SimpleObject_(CreateBitmap(gfx, hIcon)) {
}

Bitmap::Bitmap(const Graphics& gfx, SizeU size)
    : SimpleObject_(CreateBitmap(gfx, size)) {
}

Bitmap::Bitmap(const Graphics& gfx)
    : Bitmap(gfx, gfx->GetPixelSize()) {
}

//////////////////////////////////////////////////////////////////////
//
// Brush
//
Brush::Brush(const Graphics& gfx, ColorF color)
    : SimpleObject_(CreateSolidColorBrush(gfx, color)) {
}

Brush::Brush(const Graphics& gfx, float red, float green, float blue,
             float alpha)
    : SimpleObject_(CreateSolidColorBrush(gfx,
                                          ColorF(red, green, blue, alpha))) {
}

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
  ASSERT(!pixels_per_dip_.empty());
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

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
FontFace::FontFace(const char16* family_name)
    : SimpleObject_(CreateFontFace(family_name)),
      metrics_(GetFontMetrics(*this)) {
}

//////////////////////////////////////////////////////////////////////
//
// Graphics
//
Graphics::Graphics(ID2D1RenderTarget* render_target)
    : batch_nesting_level_(0),
      factory_set_(FactorySet::instance()),
      hwnd_(nullptr),
      render_target_(render_target),
      work_(nullptr) {
  if (render_target_) {
    SizeF dpi;
    render_target_->GetDpi(&dpi.width, &dpi.height);
    UpdateDpi(dpi);
  }
}

Graphics::Graphics(Graphics&& other)
    : batch_nesting_level_(0),
      factory_set_(std::move(other.factory_set_)),
      hwnd_(other.hwnd_),
      render_target_(std::move(other.render_target_)),
      work_(nullptr) {
  other.hwnd_ = nullptr;
}

Graphics::Graphics() : Graphics(nullptr) {
}

Graphics::~Graphics() {
}

Graphics& Graphics::operator=(Graphics&& other) {
  factory_set_ = std::move(other.factory_set_);
  render_target_ = std::move(other.render_target_);
  hwnd_ = other.hwnd_;
  other.hwnd_ = nullptr;
  return *this;
}

void Graphics::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void Graphics::BeginDraw() const {
  #if DEBUG_DRAW
    DEBUG_PRINTF("%p nesting=%d\n", render_target_, batch_nesting_level_);
  #endif
  ASSERT(render_target_);
  if (!batch_nesting_level_)
    render_target_->BeginDraw();
  ++batch_nesting_level_;
}

Graphics Graphics::CreateCompatible(const SizeF& size) const {
  common::ComPtr<ID2D1BitmapRenderTarget> compatible_target;
  auto const hr = render_target_->CreateCompatibleRenderTarget(
    size, &compatible_target);
  if (FAILED(hr)) {
    DVLOG(0) << "CreateCompatibleRenderTarget: hr=0x" << std::hex << hr;
    return Graphics();
  }
  common::ComPtr<IDWriteRenderingParams> params;
  render_target_->GetTextRenderingParams(&params);
  if (params) {
    compatible_target->SetTextRenderingParams(params);
    DVLOG(0) << "ClearTypeLevel=" << params->GetClearTypeLevel() <<
                " Gamma=" << params->GetGamma() <<
                " PixelGeometry=" << params->GetPixelGeometry() <<
                " RenderingMode=" << params->GetRenderingMode();
  }
  DVLOG(0) << "Before AntialiasMode=" <<
    compatible_target->GetTextAntialiasMode();
  auto const antialias_mode = render_target_->GetTextAntialiasMode();
  compatible_target->SetTextAntialiasMode(antialias_mode);
  DVLOG(0) << "AntialiasMode=" << antialias_mode;
  return std::move(Graphics(compatible_target.release()));
}

void Graphics::DrawBitmap(const Bitmap& bitmap, const RectF& dst_rect,
                          const RectF& src_rect, float opacity,
                          D2D1_BITMAP_INTERPOLATION_MODE mode) const {
  render_target_->DrawBitmap(bitmap, dst_rect, opacity, mode, src_rect);
}

bool Graphics::EndDraw() {
  #if DEBUG_DRAW
    DEBUG_PRINTF("%p nesting=%d\n", render_target_, batch_nesting_level_);
  #endif
  ASSERT(drawing());
  ASSERT(render_target_);
  --batch_nesting_level_;
  if (batch_nesting_level_) {
    auto const hr = render_target_->Flush();
    if (SUCCEEDED(hr))
      return true;
    if (hr == D2DERR_RECREATE_TARGET) {
      Debugger::Printf("Got D2DERR_RECREATE_TARGET\n", hr);
    } else {
      Debugger::Printf("ID2D1RenderTarget::Flush failed hr=0x%0X\n", hr);
    }
  } else {
      auto const hr = render_target_->EndDraw();
      if (SUCCEEDED(hr))
        return true;
      if (hr == D2DERR_RECREATE_TARGET) {
        Debugger::Printf("Got D2DERR_RECREATE_TARGET\n", hr);
      } else {
        Debugger::Printf("ID2D1RenderTarget::EndDraw failed hr=0x%0X\n", hr);
      }
  }
  const_cast<Graphics*>(this)->render_target_.reset();
  const_cast<Graphics*>(this)->Reinitialize();
  return false;
}

void Graphics::Flush() const {
  ASSERT(drawing());
  D2D1_TAG tag1, tag2;
  COM_VERIFY(render_target_->Flush(&tag1, &tag2));
  ASSERT(!tag1 && !tag2);
}

void Graphics::Init(HWND hwnd) {
  ASSERT(!hwnd_);
  hwnd_ = hwnd;
  Reinitialize();
}

void Graphics::Reinitialize() {
  DCHECK(!render_target_);
  DCHECK(hwnd_);
  RECT rc;
  ::GetClientRect(hwnd_, &rc);
  auto const pixel_format = D2D1::PixelFormat(
      DXGI_FORMAT_B8G8R8A8_UNORM,
      D2D1_ALPHA_MODE_PREMULTIPLIED);
  auto const size = SizeU(rc.right - rc.left, rc.bottom - rc.top);
  // TODO: When should use D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE?
  //auto const usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
  auto const usage = D2D1_RENDER_TARGET_USAGE_NONE;
  common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
  COM_VERIFY(FactorySet::d2d1().CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                   pixel_format, 0.0f, 0.0f,
                                   usage),
      D2D1::HwndRenderTargetProperties(hwnd_, size,
                                       D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
      &hwnd_render_target));
  render_target_.reset(hwnd_render_target);
  SizeF dpi;
  render_target_->GetDpi(&dpi.width, &dpi.height);
  UpdateDpi(dpi);
  render_target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
  FOR_EACH_OBSERVER(Observer, observers_, ShouldDiscardResources());
}

void Graphics::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void Graphics::Resize(const Rect& rc) const {
  SizeU size(rc.width(), rc.height());
  common::ComPtr<ID2D1HwndRenderTarget> hwnd_render_target;
  COM_VERIFY(hwnd_render_target.QueryFrom(render_target_));
  COM_VERIFY(hwnd_render_target->Resize(size));
}

//////////////////////////////////////////////////////////////////////
//
// Graphics::AxisAlignedClipScope
//
Graphics::AxisAlignedClipScope::AxisAlignedClipScope(
    const Graphics& gfx, const RectF& rect)
    : gfx_(gfx) {
  gfx_->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

Graphics::AxisAlignedClipScope::~AxisAlignedClipScope() {
  gfx_->PopAxisAlignedClip();
}

//////////////////////////////////////////////////////////////////////
//
// Graphics::Observer
//
Graphics::Observer::Observer() {
}

Graphics::Observer::~Observer() {
}

//////////////////////////////////////////////////////////////////////
//
// TextFormat
//
TextFormat::TextFormat(const base::string16& font_face_name, float font_size)
    : SimpleObject_(CreateTextFormat(font_face_name, font_size)),
      factory_set_(FactorySet::instance()) {
}

TextFormat::TextFormat(const LOGFONT& log_font)
    : SimpleObject_(CreateTextFormat(log_font.lfFaceName,
                                     pt2dip(-log_font.lfHeight))),
      factory_set_(FactorySet::instance()) {
}

} // namespace gfx
