// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_
#define EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_

#include <dwrite.h>

#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/win/scoped_comptr.h"

namespace visuals {

class FloatSize;
class NativeTextLayout;

//////////////////////////////////////////////////////////////////////
//
// NativeTextFormat
//
class NativeTextFormat final {
 public:
  NativeTextFormat(const base::string16& font_face_name, float font_size);
  ~NativeTextFormat();

  const base::win::ScopedComPtr<IDWriteTextFormat>& get() const {
    return text_format_;
  }

  FloatSize ComputeMetrics(const base::string16& text) const;

 private:
  base::win::ScopedComPtr<IDWriteTextFormat> text_format_;

  DISALLOW_COPY_AND_ASSIGN(NativeTextFormat);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_NATIVE_TEXT_FORMAT_WIN_H_
