// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/components/imaging/image_data.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/ginx/array_buffer_view.h"
#include "evita/visuals/imaging/image_bitmap.h"

namespace dom {

using FloatSize = visuals::FloatSize;
using ImageBitmap = visuals::ImageBitmap;

namespace {

ImageBitmap CreateImageBitmap(const void* data,
                              size_t data_size,
                              int width,
                              int height) {
  if (width <= 0 || height <= 0)
    return ImageBitmap();
  const auto stride = width * 4;
  if (data_size != static_cast<size_t>(stride * height))
    return ImageBitmap();
  return ImageBitmap(data, data_size, FloatSize(width, height));
}

ImageBitmap CreateImageBitmap(int width, int height) {
  if (width <= 0 || height <= 0)
    return ImageBitmap();
  return ImageBitmap(FloatSize(width, height));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
ImageData::ImageData(const ImageBitmap& bitmap) : bitmap_(bitmap) {}

ImageData::ImageData(const gin::ArrayBufferView& input,
                     int width,
                     int height,
                     ExceptionState* exception_state)
    : bitmap_(std::move(
          CreateImageBitmap(input.bytes(), input.num_bytes(), width, height))) {
  if (bitmap_.is_valid())
    return;
  exception_state->ThrowError("Failed to create ImageData");
}

ImageData::ImageData(int width, int height, ExceptionState* exception_state)
    : bitmap_(std::move(CreateImageBitmap(width, height))) {
  if (bitmap_.is_valid())
    return;
  exception_state->ThrowError("Failed to create ImageData");
}

ImageData::~ImageData() {}

std::vector<uint8_t> ImageData::data() const {
  return bitmap_.data();
}

int ImageData::height() const {
  return static_cast<int>(bitmap_.size().height());
}

int ImageData::width() const {
  return static_cast<int>(bitmap_.size().width());
}

// static
ImageData* ImageData::Decode(const base::string16& mime_type,
                             const gin::ArrayBufferView& data,
                             ExceptionState* exception_state) {
  const auto& bitmap =
      ImageBitmap::Decode(mime_type, data.bytes(), data.num_bytes());
  if (!bitmap.is_valid()) {
    exception_state->ThrowError("Failed to decode data");
    return nullptr;
  }
  return new ImageData(bitmap);
}

std::vector<uint8_t> ImageData::Encode(const base::string16& mime_type,
                                       ExceptionState* exception_state) const {
  const auto& data = bitmap_.Encode(mime_type);
  if (data.empty())
    exception_state->ThrowError("Failed to encode data");
  return data;
}

}  // namespace dom
