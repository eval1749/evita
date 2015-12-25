// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <memory>

#include "base/macros.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/inline_block_box.h"
#include "evita/visuals/model/text_box.h"

namespace visuals {

class BoxPrinter final : public BoxVisitor {
 public:
  BoxPrinter() = default;
  ~BoxPrinter() final = default;

 private:
  void Indent() const;

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  int indent_ = 0;

  DISALLOW_COPY_AND_ASSIGN(BoxPrinter);
};

void BoxPrinter::Indent() const {
  for (auto i = indent_; i > 0; --i)
    std::cout << "  ";
}

void BoxPrinter::VisitBlockBox(BlockBox* box) {
  Indent();
  std::cout << *box << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes()) {
    Visit(child);
    std::cout << std::endl;
  }
  --indent_;
}

void BoxPrinter::VisitInlineBlockBox(InlineBlockBox* box) {
  Indent();
  std::cout << *box << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes()) {
    Visit(child);
    std::cout << std::endl;
  }
  --indent_;
}

void BoxPrinter::VisitTextBox(TextBox* box) {
  Indent();
  std::cout << *box;
}

void DemoMain() {
  auto root_box = std::make_unique<BlockBox>(FloatRect(FloatSize(640, 480)),
                                             FloatRect(FloatSize(640, 480)));
  auto text_box1 = std::make_unique<TextBox>(
      FloatRect(FloatPoint(10, 10), FloatSize(100, 20)), L"foo");
  auto text_box2 = std::make_unique<TextBox>(
      FloatRect(FloatPoint(10, 40), FloatSize(100, 20)), L"bar");
  root_box->AppendChild(text_box1.get());
  root_box->AppendChild(text_box2.get());

  BoxPrinter printer;
  printer.Visit(*root_box);
}

}  // namespace visuals

void main() {
  visuals::DemoMain();
}
