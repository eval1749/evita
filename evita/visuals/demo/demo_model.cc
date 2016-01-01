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
#include "evita/visuals/model/box_traversal.h"
#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/line_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"

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
  void PrintAsContainer(const ContainerBox& container);

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

void BoxPrinter::PrintAsContainer(const ContainerBox& container) {
  Indent();
  const auto& style = container.ComputeActualStyle();
  std::cout << container << ' ' << *style << std::endl;
  ++indent_;
  for (const auto& child : container.child_boxes())
    Visit(child);
  --indent_;
}

// BoxVisitor
void BoxPrinter::VisitBlockBox(BlockBox* box) {
  PrintAsContainer(*box);
}

void BoxPrinter::VisitLineBox(LineBox* box) {
  PrintAsContainer(*box);
}

void BoxPrinter::VisitRootBox(RootBox* box) {
  PrintAsContainer(*box);
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

const auto kMargin = 8;
const auto kBorder = 1;

std::unique_ptr<RootBox> CreateBoxTree() {
  BoxBuilder root(BoxBuilder::New<RootBox>());
  BoxBuilder body(BoxBuilder::New<BlockBox>());
  body.SetStyle(*css::StyleBuilder()
                     .SetBackground(css::Background(css::Color(1, 1, 1)))
                     .SetPadding(css::Padding(kMargin, kMargin, kMargin, 0))
                     .Build());
  BoxBuilder list(BoxBuilder::New<BlockBox>());
  const auto& kBlack =
      css::StyleBuilder().SetColor(css::Color(0, 0, 0)).Build();
  for (auto index = 0; index < 20; ++index) {
    BoxBuilder line(BoxBuilder::New<LineBox>());
    line.SetStyle(
            *css::StyleBuilder()
                 .SetBorder(css::Border(index & 1 ? css::Color(0, 0.5f, 0)
                                                  : css::Color(0, 0, 0.5f),
                                        kBorder))
                 .Build())
        .Append(BoxBuilder::New<TextBox>(base::StringPrintf(L"line %d", index))
                    .SetStyle(*kBlack)
                    .Finish())
        .Append(BoxBuilder::New<TextBox>(L"size").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"status").SetStyle(*kBlack).Finish())
        .Append(BoxBuilder::New<TextBox>(L"file").SetStyle(*kBlack).Finish());
    switch (index) {
      case 0:
        line.SetStyle(
            *css::StyleBuilder().SetDisplay(css::Display::None()).Build());
        break;
      case 2:
        // Selected color
        line.SetStyle(*css::StyleBuilder()
                           .SetBackground(css::Background(
                               css::Color::Rgba(51, 153, 255, 0.5f)))
                           .Build());
        break;
      case 3:
        // Inactive selection color
        line.SetStyle(*css::StyleBuilder()
                           .SetBackground(css::Background(
                               css::Color::Rgba(191, 205, 191, 0.2f)))
                           .Build());
        break;
    }
    list.Append(line.Finish());
  }
  BoxBuilder hover(BoxBuilder::New<LineBox>(L"hover"));
  hover.SetStyle(
           *css::StyleBuilder()
                .SetPosition(css::Position::Absolute())
                .SetLeft(css::Left(css::Length(20)))
                .SetTop(css::Top(css::Length(120)))
                .SetBackground(
                    css::Background(css::Color::Rgba(51, 153, 255, 0.1f)))
                .SetBorder(css::Border(css::Color::Rgba(51, 153, 255, 1.0f), 1))
                .Build())
      .Append(BoxBuilder::New<TextBox>(L"hover").SetStyle(*kBlack).Finish());
  list.Append(hover.Finish());
  body.Append(list.Finish());
  root.Append(body.Finish());
  return std::unique_ptr<RootBox>(root.Finish().release()->as<RootBox>());
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
DemoModel::DemoModel() : root_box_(CreateBoxTree()) {}
DemoModel::~DemoModel() {}

void DemoModel::AttachWindow(DemoWindow* window) {
  window_ = window;
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void DemoModel::DidBeginAnimationFrame(base::Time now) {
  const auto& canvas = window_->GetCanvas();
  if (!canvas)
    return RequestAnimationFrame();

  Layouter().Layout(root_box_.get());
  if (root_box_->IsPaintClean())
    return;
  PaintInfo paint_info(FloatRect(root_box_->viewport_size()));
  auto list = Painter().Paint(paint_info, *root_box_);

#if 0
  static base::Time last_time;
  if (now - last_time >= base::TimeDelta::FromMilliseconds(1000)) {
    last_time = now;
    PrintBox(*root_box_);
    PrintPaint(*list);
  }
#endif

  DisplayItemListProcessor processor;
  processor.Paint(canvas, std::move(list));
}

// WindowEventHandler
void DemoModel::DidChangeWindowBounds(const FloatRect& bounds) {
  RequestAnimationFrame();
  BoxEditor().SetViewportSize(root_box_.get(), bounds.size());
}

void DemoModel::DidMoveMouse(const FloatPoint& point) {
  const auto hover = root_box_->GetBoxById(L"hover");
  // TODO(eval1749): We should have a mapping function from pageX/pageY to
  // boxX/boxY.
  const auto hover_y = point.y() - kMargin - kBorder;
  BoxEditor().SetStyle(
      hover,
      *css::StyleBuilder().SetTop(css::Top(css::Length(hover_y))).Build());
  RequestAnimationFrame();
}

const char* DemoModel::GetAnimationFrameType() const {
  return "DemoModel";
}

}  // namespace visuals
