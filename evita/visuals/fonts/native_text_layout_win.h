// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_
#define EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_

#include <dwrite.h>

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/win/scoped_comptr.h"

namespace visuals {

class FloatSize;
class NativeTextFormat;

//////////////////////////////////////////////////////////////////////
//
// NativeTextLayout
//
class NativeTextLayout final {
 public:
  NativeTextLayout(const NativeTextFormat& text_format,
                   const base::string16& text,
                   const FloatSize& size);
  ~NativeTextLayout();

  const base::win::ScopedComPtr<IDWriteTextLayout>& get() const {
    return text_layout_;
  }

  FloatSize GetMetrics() const;

 private:
  base::win::ScopedComPtr<IDWriteTextLayout> text_layout_;

  DISALLOW_COPY_AND_ASSIGN(NativeTextLayout);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_
