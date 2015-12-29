// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "evita/visuals/demo/demo_model.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/style/style.h"
#include "evita/visuals/style/style_builder.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// BoxPrinter
//
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
  const auto& style = box->ComputeActualStyle();
  std::cout << *box << ' ' << *style << std::endl;
  ++indent_;
  for (const auto& child : box->child_boxes())
    Visit(child);
  --indent_;
}

void BoxPrinter::VisitLineBox(LineBox* box) {
  Indent();
  const auto& style = box->ComputeActualStyle();
  std::cout << *box << ' ' << *style << std::endl;
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
  const auto& style = box->ComputeActualStyle();
  std::cout << '"' << ' ' << *style << std::endl;
}

std::unique_ptr<Box> CreateRootBox() {
  // |root| should provide background color. Note: |root| can't have border
  // and margin.
  BoxBuilder root(BoxBuilder::New<BlockBox>());
  root.SetStyle(*StyleBuilder()
                     .SetBackground(Background(FloatColor(1, 1, 1)))
                     .SetPadding(Padding(8, 8, 8, 0))
                     .Build());
  const auto& kBlack = StyleBuilder().SetColor(FloatColor(0, 0, 0)).Build();
  for (auto index = 0; index < 15; ++index) {
    BoxBuilder line(BoxBuilder::New<LineBox>());
    line.SetStyle(*StyleBuilder()
                       .SetBorder(Border(index & 1 ? FloatColor(0, 0.5f, 0)
                                                   : FloatColor(0, 0, 0.5f),
                                         2))
                       .SetBackground(
                           Background(index & 1 ? FloatColor(0.9f, 0.9f, 0.9f)
                                                : FloatColor(1, 1, 1)))
                       .Build())
        .Append(BoxBuilder::New<TextBox>(base::StringPrintf(L"line %d", index))
                    .SetStyle(*kBlack)
                    .Finish())
        .Append(BoxBuilder::New<TextBox>(L"size").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"status").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"file").SetStyle(*kBlack).Finish());
    if (index == 0)
      line.SetStyle(*StyleBuilder().SetDisplay(Display::None).Build());
    root.Append(line.Finish());
  }

  return root.Finish();
}

void PrintBox(const Box& box) {
  std::cout << "Box Tree:" << std::endl;
  BoxPrinter printer;
  printer.Visit(box);
}

void PrintPaint(const DisplayItemList& list) {
  std::cout << std::endl << "Display Item List:" << std::endl;
  auto indent = 0;
  for (const auto& item : list.items()) {
    if (item->is<EndClipDisplayItem>() || item->is<EndTransformDisplayItem>()) {
      --indent;
    }
    for (auto count = indent; count > 0; --count)
      std::cout << "  ";
    std::cout << item << std::endl;
    if (item->is<BeginClipDisplayItem>() ||
        item->is<BeginTransformDisplayItem>()) {
      ++indent;
    }
  }

  std::cout << std::endl << "Damage rects:" << std::endl;
  for (const auto& rect : list.rects())
    std::cout << rect << std::endl;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
/// DemoModel
//
DemoModel::DemoModel() : root_box_(CreateRootBox()) {}
DemoModel::~DemoModel() {}

void DemoModel::AttachWindow(DemoWindow* window) {
  window_ = window;
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void DemoModel::DidBeginAnimationFrame(base::Time now) {
  static base::Time last_time;

  const auto& canvas = window_->GetCanvas();
  if (!canvas)
    return RequestAnimationFrame();

  const auto& viewport_bounds = FloatRect(
      FloatSize(window_->bounds().width(), window_->bounds().height()));
  Layouter().Layout(root_box_.get(), viewport_bounds);

  PaintInfo paint_info(viewport_bounds);
  auto list = Painter().Paint(paint_info, *root_box_);

  if (now - last_time >= base::TimeDelta::FromMilliseconds(1000)) {
    last_time = now;
    PrintBox(*root_box_);
    PrintPaint(*list);
  }

  DisplayItemListProcessor processor;
  processor.Paint(canvas, std::move(list));
}

// WindowEventHandler
void DemoModel::DidChangeWindowBounds(const FloatRect& bounds) {
  RequestAnimationFrame();
}

const char* DemoModel::GetAnimationFrameType() const {
  return "DemoModel";
}

}  // namespace visuals
