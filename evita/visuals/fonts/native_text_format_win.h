// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_
#define EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_

#include <dwrite.h>

#include <wrl/client.h>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace gfx {
class FloatSize;
}

namespace visuals {

class FontDescription;
class NativeTextLayout;

//////////////////////////////////////////////////////////////////////
//
// NativeTextFormat
//
class NativeTextFormat final {
 public:
  explicit NativeTextFormat(const FontDescription& description);
  ~NativeTextFormat();

  const Microsoft::WRL::ComPtr<IDWriteTextFormat>& get() const {
    return text_format_;
  }

  gfx::FloatSize ComputeMetrics(const base::string16& text) const;

 private:
  Microsoft::WRL::ComPtr<IDWriteTextFormat> text_format_;

  DISALLOW_COPY_AND_ASSIGN(NativeTextFormat);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_
