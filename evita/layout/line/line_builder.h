// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_LINE_BUILDER_H_
#define EVITA_LAYOUT_LINE_LINE_BUILDER_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/text/offset.h"

namespace layout {

class InlineBox;
class RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  explicit LineBuilder(text::Offset text_start);
  ~LineBuilder();

  float current_x() const { return current_x_; }
  InlineBox* last_box() const { return boxes_.back(); }

  void Add(InlineBox* inline_box);
  scoped_refptr<RootInlineBox> Build(text::Offset text_end);

 private:
  float ascent_;
  std::vector<InlineBox*> boxes_;
  float descent_;
  float current_x_;
  float last_box_width_;
  const text::Offset text_start_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_LINE_BUILDER_H_
