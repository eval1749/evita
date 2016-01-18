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
class FloatRect;
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
  NativeTextLayout(const NativeTextLayout& other);
  NativeTextLayout(NativeTextLayout&& other);
  ~NativeTextLayout();

  bool operator==(const NativeTextLayout& other) const;
  bool operator!=(const NativeTextLayout& other) const;

  const base::win::ScopedComPtr<IDWriteTextLayout>& get() const {
    return value_;
  }

  FloatSize GetMetrics() const;
  FloatRect HitTestTextPosition(size_t offset) const;

 private:
  base::win::ScopedComPtr<IDWriteTextLayout> value_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_
