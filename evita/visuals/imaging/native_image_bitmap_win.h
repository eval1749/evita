// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_
#define EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_

#include <stdint.h>
#include <wincodec.h>

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_gdi_object.h"
#include "evita/gfx/base/geometry/float_size.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NativeImageBitmap
//
class NativeImageBitmap final {
 public:
  NativeImageBitmap(const void* data,
                    size_t data_size,
                    const gfx::FloatSize& size);
  explicit NativeImageBitmap(base::win::ScopedComPtr<IWICBitmapSource>&& data);
  explicit NativeImageBitmap(
      const base::win::ScopedComPtr<IWICBitmapSource>& data);
  explicit NativeImageBitmap(const base::win::ScopedHICON& icon);
  explicit NativeImageBitmap(const gfx::FloatSize& size);
  NativeImageBitmap(const NativeImageBitmap& other);
  NativeImageBitmap(NativeImageBitmap&& other);
  ~NativeImageBitmap();

  NativeImageBitmap& operator=(const NativeImageBitmap& other);
  NativeImageBitmap& operator=(NativeImageBitmap&& other);

  bool operator==(const NativeImageBitmap& other) const;
  bool operator!=(const NativeImageBitmap& other) const;

  std::vector<uint8_t> data() const;
  base::string16 format() const;
  const base::win::ScopedComPtr<IWICBitmapSource>& get() const { return impl_; }
  gfx::FloatSize resolution() const;
  const gfx::FloatSize& size() const { return size_; }

  static NativeImageBitmap Decode(base::StringPiece16 format,
                                  const void* data,
                                  size_t data_size);

  std::vector<uint8_t> Encode(base::StringPiece16 format) const;

 private:
  NativeImageBitmap();

  base::win::ScopedComPtr<IWICBitmapSource> impl_;
  gfx::FloatSize size_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_
