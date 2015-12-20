// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_LINE_BUILDER_H_
#define EVITA_LAYOUT_LINE_LINE_BUILDER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/layout/render_style.h"
#include "evita/text/offset.h"

namespace layout {

class InlineBox;
class RenderStyle;
class RootInlineBox;
enum class TextMarker;

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  LineBuilder(const RenderStyle& style,
              text::Offset line_start,
              text::Offset text_start,
              float line_width);
  ~LineBuilder();

  float current_x() const { return current_x_; }

  void AddCodeUnitBox(const RenderStyle& style,
                      float width,
                      float height,
                      text::Offset offset,
                      const base::string16& text);
  void AddFillerBox(const RenderStyle& style,
                    float width,
                    float height,
                    text::Offset offset);
  void AddMarkerBox(const RenderStyle& style,
                    float width,
                    float height,
                    text::Offset start,
                    text::Offset end,
                    TextMarker marker_name);
  void AddTextBoxIfNeeded();
  std::unique_ptr<RootInlineBox> Build();
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
  const text::Offset line_start_;
  RenderStyle style_;
  const text::Offset text_start_;
  float pending_text_width_ = 0.0f;
  std::vector<base::char16> pending_text_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_LINE_BUILDER_H_
