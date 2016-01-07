// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/strings/stringprintf.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/layout/box_tree.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// BoxTreePrinter
//
class BoxTreePrinter final : public BoxVisitor {
 public:
  explicit BoxTreePrinter(std::ostream* ostream);
  ~BoxTreePrinter() final = default;

 private:
  void Indent() const;
  void PrintAsContainer(const ContainerBox& container);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  int indent_ = 0;
  std::ostream* const ostream_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreePrinter);
};

BoxTreePrinter::BoxTreePrinter(std::ostream* ostream) : ostream_(ostream) {}

void BoxTreePrinter::Indent() const {
  for (auto i = indent_; i > 0; --i)
    *ostream_ << "  ";
}

void BoxTreePrinter::PrintAsContainer(const ContainerBox& container) {
  Indent();
  const auto& style = container.ComputeActualStyle();
  *ostream_ << container << ' ' << *style << std::endl;
  ++indent_;
  for (const auto& child : container.child_boxes())
    Visit(child);
  --indent_;
}

// BoxVisitor
void BoxTreePrinter::VisitFlowBox(FlowBox* box) {
  PrintAsContainer(*box);
}

void BoxTreePrinter::VisitRootBox(RootBox* box) {
  PrintAsContainer(*box);
}

void BoxTreePrinter::VisitTextBox(TextBox* box) {
  Indent();
  *ostream_ << *box << " \"";
  for (const auto& char_code : box->text()) {
    if (char_code < 0x20 || char_code >= 0x7F) {
      *ostream_ << base::StringPrintf("\\u%04X", char_code);
      continue;
    }
    if (char_code == '"' || char_code == '\\')
      *ostream_ << '\\';
    *ostream_ << static_cast<char>(char_code);
  }
  const auto& style = box->ComputeActualStyle();
  *ostream_ << '"' << ' ' << *style << std::endl;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const BoxTree& box_tree) {
  BoxTreePrinter(&ostream).Visit(box_tree.root_box());
  return ostream;
}

}  // namespace visuals
