// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_LINE_BUILDER_H_
#define EVITA_LAYOUT_LINE_LINE_BUILDER_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/memory/ref_counted.h"
#include "evita/layout/render_style.h"
#include "evita/text/offset.h"

namespace layout {

class InlineBox;
class RenderStyle;
class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  explicit LineBuilder(const RenderStyle& style,
                       text::Offset text_start,
                       float line_width);
  ~LineBuilder();

  float current_x() const { return current_x_; }

  void AddBox(InlineBox* inline_box);
  void AddTextBoxIfNeeded();
  scoped_refptr<RootInlineBox> Build(text::Offset text_end);
  bool HasRoomFor(float width) const;
  bool TryAddChar(const RenderStyle& style,
                  text::Offset offset,
                  base::char16 char_code);

 private:
  void AddBoxInternal(InlineBox* inline_box);

  float ascent_ = 0.0f;
  std::vector<InlineBox*> boxes_;
  float descent_ = 0.0f;
  text::Offset current_offset_;
  float current_x_ = 0.0f;
  const float line_width_;
  RenderStyle style_;
  const text::Offset text_start_;
  float pending_text_width_ = 0.0f;
  std::vector<base::char16> pending_text_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_LINE_BUILDER_H_
