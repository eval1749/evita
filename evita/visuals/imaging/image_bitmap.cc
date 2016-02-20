// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/visuals/imaging/image_bitmap.h"

#include "build/build_config.h"

#if OS_WIN
#include "evita/visuals/imaging/native_image_bitmap_win.h"
#else
#error "Unsupported target"
#endif

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageBitmap
//
ImageBitmap::ImageBitmap(const void* data,
                         size_t data_size,
                         const FloatSize& size)
    : impl_(new NativeImageBitmap(data, data_size, size)) {
  if (impl_->get())
    return;
  impl_.reset();
}

ImageBitmap::ImageBitmap(std::unique_ptr<NativeImageBitmap> impl)
    : impl_(std::move(impl)) {
  if (impl_->get())
    return;
  impl_.reset();
}

ImageBitmap::ImageBitmap(const FloatSize& size)
    : impl_(new NativeImageBitmap(size)) {}

ImageBitmap::ImageBitmap(const ImageBitmap& other)
    : impl_(new NativeImageBitmap(*other.impl_)) {}

ImageBitmap::ImageBitmap(ImageBitmap&& other) : impl_(std::move(other.impl_)) {}

ImageBitmap::ImageBitmap() {}
ImageBitmap::~ImageBitmap() {}

ImageBitmap& ImageBitmap::operator=(const ImageBitmap& other) {
  impl_.reset(new NativeImageBitmap(*other.impl_));
  return *this;
}

ImageBitmap& ImageBitmap::operator=(ImageBitmap&& other) {
  impl_ = std::move(other.impl_);
  return *this;
}

bool ImageBitmap::operator==(const ImageBitmap& other) const {
  return *impl_ == *other.impl_;
}

bool ImageBitmap::operator!=(const ImageBitmap& other) const {
  return !operator==(other);
}

std::vector<uint8_t> ImageBitmap::data() const {
  return impl_->data();
}

base::string16 ImageBitmap::format() const {
  return impl_->format();
}

FloatSize ImageBitmap::resolution() const {
  return impl_->resolution();
}

const FloatSize& ImageBitmap::size() const {
  return impl_->size();
}

// static
ImageBitmap ImageBitmap::Decode(base::StringPiece16 format,
                                const void* data,
                                size_t data_size) {
  return ImageBitmap(std::move(std::make_unique<NativeImageBitmap>(
      std::move(NativeImageBitmap::Decode(format, data, data_size)))));
}

std::vector<uint8_t> ImageBitmap::Encode(base::StringPiece16 format) const {
  return impl_->Encode(format);
}

std::ostream& operator<<(std::ostream& ostream, const ImageBitmap& image) {
  return ostream << "ImageBitmap(" << &image.impl() << ' ' << image.size()
                 << ')';
}

}  // namespace visuals
