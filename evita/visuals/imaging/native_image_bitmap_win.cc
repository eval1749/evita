// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/imaging/native_image_bitmap_win.h"

#include <unordered_map>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "common/win/com_verify.h"
#include "evita/gfx/imaging_factory_win.h"

namespace visuals {

namespace {

int ExtractSizeParameter(base::StringPiece16 format, int default_value) {
  const auto size_pos = format.find(L";size=");
  if (size_pos == std::string::npos)
    return default_value;
  int value = 0;
  if (base::StringToInt(format.substr(size_pos + 6), &value))
    return value;
  return default_value;
}

base::StringPiece16 ExtractType(base::StringPiece16 format) {
  const auto semi_colon = format.find(';');
  if (semi_colon == std::string::npos)
    return format;
  return format.substr(0, semi_colon);
}

//////////////////////////////////////////////////////////////////////
//
// ImageFormatMap
//
class ImageFormatMap final {
 public:
  ImageFormatMap();
  ~ImageFormatMap() = default;

  GUID Get(base::StringPiece16 format) const;

  static ImageFormatMap* GetInstance();

 private:
  std::unordered_map<base::StringPiece16, GUID, base::StringPiece16Hash> map_;
  DISALLOW_COPY_AND_ASSIGN(ImageFormatMap);
};

ImageFormatMap::ImageFormatMap() {
  map_.emplace(L"image/bmp", GUID_ContainerFormatBmp);
  map_.emplace(L"image/gif", GUID_ContainerFormatGif);
  map_.emplace(L"image/ico", GUID_ContainerFormatIco);
  map_.emplace(L"image/jpeg", GUID_ContainerFormatJpeg);
  map_.emplace(L"image/jpe", GUID_ContainerFormatJpeg);
  map_.emplace(L"image/jpg", GUID_ContainerFormatJpeg);
  map_.emplace(L"image/png", GUID_ContainerFormatPng);
  map_.emplace(L"image/tiff", GUID_ContainerFormatTiff);
}

GUID ImageFormatMap::Get(base::StringPiece16 format) const {
  const auto& it = map_.find(ExtractType(format));
  return it == map_.end() ? GUID_NULL : it->second;
}

// static
ImageFormatMap* ImageFormatMap::GetInstance() {
  return base::Singleton<ImageFormatMap>::get();
}

gfx::FloatSize ComputeSize(
    const base::win::ScopedComPtr<IWICBitmapSource>& bitmap) {
  uint32_t width = 0;
  uint32_t height = 0;
  COM_VERIFY(bitmap->GetSize(&width, &height));
  return gfx::FloatSize(static_cast<float>(width), static_cast<float>(height));
}

base::win::ScopedComPtr<IWICBitmapSource>
CreateBitmap(const void* data, size_t data_size, const gfx::FloatSize& size) {
  base::win::ScopedComPtr<IWICBitmap> bitmap;
  COM_VERIFY2(
      gfx::ImagingFactory::GetInstance()->impl()->CreateBitmap(
          static_cast<uint32_t>(size.width()),
          static_cast<uint32_t>(size.height()), GUID_WICPixelFormat32bppRGBA,
          WICBitmapCacheOnDemand, bitmap.Receive()),
      base::win::ScopedComPtr<IWICBitmapSource>());
  {
    base::win::ScopedComPtr<IWICBitmapLock> lock;
    COM_VERIFY2(bitmap->Lock(nullptr, WICBitmapLockRead, lock.Receive()),
                base::win::ScopedComPtr<IWICBitmapSource>());
    uint8_t* pixels = nullptr;
    uint32_t pixels_size = 0;
    COM_VERIFY2(lock->GetDataPointer(&pixels_size, &pixels),
                base::win::ScopedComPtr<IWICBitmapSource>());
    if (pixels_size != data_size) {
      DCHECK_EQ(pixels_size, data_size);
      return base::win::ScopedComPtr<IWICBitmapSource>();
    }
    ::memcpy(pixels, data, pixels_size);
  }
  return base::win::ScopedComPtr<IWICBitmapSource>(bitmap.get());
}

base::win::ScopedComPtr<IWICBitmapSource> CreateBitmap(
    const gfx::FloatSize& size) {
  base::win::ScopedComPtr<IWICBitmap> bitmap;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateBitmap(
      static_cast<uint32_t>(size.width()), static_cast<uint32_t>(size.height()),
      GUID_WICPixelFormat32bppRGBA, WICBitmapCacheOnDemand, bitmap.Receive()));
  return base::win::ScopedComPtr<IWICBitmapSource>(bitmap.get());
}

base::win::ScopedComPtr<IWICBitmapSource> CreateBitmapFromIcon(
    const base::win::ScopedHICON& icon) {
  base::win::ScopedComPtr<IWICBitmap> bitmap;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateBitmapFromHICON(
      icon.get(), bitmap.Receive()));
  return base::win::ScopedComPtr<IWICBitmapSource>(bitmap.get());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NativeImageBitmap
//
NativeImageBitmap::NativeImageBitmap(const void* data,
                                     size_t data_size,
                                     const gfx::FloatSize& size)
    : impl_(CreateBitmap(data, data_size, size)), size_(size) {}

NativeImageBitmap::NativeImageBitmap(
    const base::win::ScopedComPtr<IWICBitmapSource>& impl)
    : impl_(impl), size_(ComputeSize(impl_)) {}

NativeImageBitmap::NativeImageBitmap(
    base::win::ScopedComPtr<IWICBitmapSource>&& impl)
    : impl_(std::move(impl)), size_(ComputeSize(impl_)) {}

NativeImageBitmap::NativeImageBitmap(const gfx::FloatSize& size)
    : impl_(std::move(CreateBitmap(size))), size_(size) {}

NativeImageBitmap::NativeImageBitmap(const base::win::ScopedHICON& icon)
    : NativeImageBitmap(CreateBitmapFromIcon(icon)) {}

NativeImageBitmap::NativeImageBitmap(const NativeImageBitmap& other)
    : impl_(other.impl_), size_(other.size_) {}

NativeImageBitmap::NativeImageBitmap(NativeImageBitmap&& other)
    : impl_(std::move(other.impl_)), size_(other.size_) {}

NativeImageBitmap::NativeImageBitmap() {}
NativeImageBitmap::~NativeImageBitmap() {}

NativeImageBitmap& NativeImageBitmap::operator=(
    const NativeImageBitmap& other) {
  impl_ = other.impl_;
  size_ = other.size_;
  return *this;
}

NativeImageBitmap& NativeImageBitmap::operator=(NativeImageBitmap&& other) {
  impl_ = std::move(other.impl_);
  size_ = other.size_;
  return *this;
}

bool NativeImageBitmap::operator==(const NativeImageBitmap& other) const {
  if (impl_ != other.impl_)
    return false;
  DCHECK_EQ(size_, other.size_);
  return true;
}

bool NativeImageBitmap::operator!=(const NativeImageBitmap& other) const {
  return !operator==(other);
}

std::vector<uint8_t> NativeImageBitmap::data() const {
  base::win::ScopedComPtr<IWICFormatConverter> converter;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateFormatConverter(
      converter.Receive()));
  const auto palette = static_cast<IWICPalette*>(nullptr);
  const auto alpha_threshold = 0.0f;
  COM_VERIFY(converter->Initialize(
      impl_.get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
      palette, alpha_threshold, WICBitmapPaletteTypeMedianCut));
  const auto stride = static_cast<uint32_t>(size_.width()) * 4;
  std::vector<uint8_t> pixels(static_cast<size_t>(stride * size_.height()));
  COM_VERIFY(converter->CopyPixels(
      nullptr, stride, static_cast<uint32_t>(pixels.size()), pixels.data()));
  return pixels;
}

base::string16 NativeImageBitmap::format() const {
  WICPixelFormatGUID guid;
  COM_VERIFY(impl_->GetPixelFormat(&guid));
  return base::StringPrintf(
      L"%08x-%04x-%04x-%02x%02x%02x%02%02x%02x%02x%02x", guid.Data1, guid.Data2,
      guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
      guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

gfx::FloatSize NativeImageBitmap::resolution() const {
  double dpi_x = 0;
  double dpi_y = 0;
  COM_VERIFY(impl_->GetResolution(&dpi_x, &dpi_y));
  return gfx::FloatSize(static_cast<float>(dpi_x), static_cast<float>(dpi_y));
}

// static
NativeImageBitmap NativeImageBitmap::Decode(base::StringPiece16 format,
                                            const void* data,
                                            size_t data_size) {
  const auto& format_guid = ImageFormatMap::GetInstance()->Get(format);
  if (format_guid == GUID_NULL)
    return NativeImageBitmap();
  if (format_guid == GUID_ContainerFormatIco) {
    const auto icon_size = ExtractSizeParameter(format, 16);
    const auto kIconVersion = 0x30000;
    const auto& icon_handle = base::win::ScopedHICON(::CreateIconFromResourceEx(
        static_cast<uint8_t*>(const_cast<void*>(data)),
        static_cast<uint32_t>(data_size),
        true,  // fIcon
        kIconVersion, icon_size, icon_size, LR_DEFAULTCOLOR));
    if (!icon_handle.is_valid()) {
      const auto last_error = ::GetLastError();
      DVLOG(0) << "CreateIconFromResourceEx error=" << last_error;
      return NativeImageBitmap();
    }
    return NativeImageBitmap(icon_handle);
  }
  base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
  const auto vendor_guid = static_cast<GUID*>(nullptr);
  COM_VERIFY2(gfx::ImagingFactory::GetInstance()->impl()->CreateDecoder(
                  format_guid, vendor_guid, decoder.Receive()),
              NativeImageBitmap());
  base::win::ScopedComPtr<IWICStream> stream;
  COM_VERIFY2(gfx::ImagingFactory::GetInstance()->impl()->CreateStream(
                  stream.Receive()),
              NativeImageBitmap());
  COM_VERIFY2(stream->InitializeFromMemory(
                  reinterpret_cast<uint8_t*>(const_cast<void*>(data)),
                  static_cast<uint32_t>(data_size)),
              NativeImageBitmap());
  COM_VERIFY2(decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand),
              NativeImageBitmap());
  uint32_t frame_count;
  COM_VERIFY2(decoder->GetFrameCount(&frame_count), NativeImageBitmap());
  if (frame_count == 0)
    return NativeImageBitmap();
  base::win::ScopedComPtr<IWICBitmapFrameDecode> frame;
  COM_VERIFY2(decoder->GetFrame(0, frame.Receive()), NativeImageBitmap());
  base::win::ScopedComPtr<IWICBitmapSource> bitmap(frame.get());
  return NativeImageBitmap(std::move(bitmap));
}

std::vector<uint8_t> NativeImageBitmap::Encode(
    base::StringPiece16 format) const {
  const auto& format_guid = ImageFormatMap::GetInstance()->Get(format);
  if (format_guid == GUID_NULL)
    return std::vector<uint8_t>();
  base::win::ScopedComPtr<IWICBitmapEncoder> encoder;
  const auto vendor_guid = static_cast<GUID*>(nullptr);
  COM_VERIFY2(gfx::ImagingFactory::GetInstance()->impl()->CreateEncoder(
                  format_guid, vendor_guid, encoder.Receive()),
              std::vector<uint8_t>());
  // TOOD(eval1749): Implement |NativeImageBitmap::Encode()|
  // 1. create stream
  // 2. encoder->Initialize(stream.get(), WICBitmapEncoderNoCache);
  // 3. encoder->CreateNewFrame(frame.Receive(), &property_bar.Receive());
  // 4. set encode options in |property_bag|
  // 5. frame->SetPiexlFormat();
  // 6. frame->WritePixels()
  // 7. frame->Commit()
  // 8. encoder->Commit()
  return std::vector<uint8_t>();
}

}  // namespace visuals
