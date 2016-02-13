// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_
#define EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_

#include <wincodec.h>

#include "base/macros.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_gdi_object.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// NativeImageBitmap
//
class NativeImageBitmap final {
 public:
  explicit NativeImageBitmap(const base::win::ScopedHICON& icon);
  ~NativeImageBitmap();

  const base::win::ScopedComPtr<IWICBitmap> get() const { return data_; }
  const FloatSize& size() const { return size_; }

 private:
  base::win::ScopedComPtr<IWICBitmap> data_;
  FloatSize size_;

  DISALLOW_COPY_AND_ASSIGN(NativeImageBitmap);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_IMAGING_NATIVE_IMAGE_BITMAP_WIN_H_
