// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
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
  for (const auto& child : box->child_boxes())
    Visit(child);
  --indent_;
}

void BoxPrinter::VisitLineBox(LineBox* box) {
  Indent();
  std::cout << *box << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes())
    Visit(child);
  --indent_;
}

void BoxPrinter::VisitTextBox(TextBox* box) {
  Indent();
  std::cout << *box << " \"";
  for (const auto& char_code : box->text()) {
    if (char_code < 0x20 || char_code >= 0x7F) {
      std::cout << base::StringPrintf("\\u%04X", char_code);
      continue;
    }
    if (char_code == '"' || char_code == '\\')
      std::cout << '\\';
    std::cout << static_cast<char>(char_code);
  }
  std::cout << '"' << std::endl;
}

void DemoMain() {
  auto root_box = std::make_unique<BlockBox>();
  for (auto index = 0; index < 10; ++index) {
    auto line_box = std::make_unique<LineBox>();
    line_box->AppendNew<TextBox>(base::StringPrintf(L"line %d", index));
    line_box->AppendNew<TextBox>(L"size");
    line_box->AppendNew<TextBox>(L"status");
    line_box->AppendNew<TextBox>(L"file");
    root_box->AppendChild(std::move(line_box));
  }
  Layouter().Layout(root_box.get(), FloatRect(FloatSize(640, 480)));

  BoxPrinter printer;
  printer.Visit(*root_box);
}

}  // namespace visuals

void main() {
  visuals::DemoMain();
}