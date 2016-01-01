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
#include "evita/visuals/model/ancestors_or_self.h"
#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_finder.h"
#include "evita/visuals/model/box_traversal.h"
#include "evita/visuals/model/box_tree_builder.h"
#include "evita/visuals/model/box_visitor.h"
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

std::unique_ptr<RootBox> BuildBoxTree() {
  auto root =
      BoxTreeBuilder()
          .Begin<BlockBox>(L"main")
          .SetStyle(*css::StyleBuilder()
                         .SetBackground(css::Background(css::Color(1, 1, 1)))
                         .SetPadding(css::Padding(kMargin, kMargin, kMargin, 0))
                         .Build())
          .Add<BlockBox>(L"list")
          .End<BlockBox>()
          .Build();
  const auto list = root->GetBoxById(L"list")->as<BlockBox>();
  const auto& kBlack =
      css::StyleBuilder().SetColor(css::Color(0, 0, 0)).Build();
  for (auto index = 0; index < 20; ++index) {
    auto line = std::make_unique<LineBox>(root.get());
    BoxTreeBuilder(line.get())
        .SetStyle(*css::StyleBuilder()
                       .SetBorder(css::Border(css::Color(), 1))
                       .Build())
        .Begin<TextBox>(base::StringPrintf(L"line %d", index))
        .SetStyle(*kBlack)
        .End<TextBox>()
        .Begin<TextBox>(L"size")
        .SetStyle(*kBlack)
        .End<TextBox>()
        .Begin<TextBox>(L"status")
        .SetStyle(*kBlack)
        .End<TextBox>()
        .Begin<TextBox>(L"file")
        .SetStyle(*kBlack)
        .End<TextBox>()
        .Finish<LineBox>(line.get());
    if (index == 0) {
      BoxEditor().SetStyle(
          line.get(),
          *css::StyleBuilder().SetDisplay(css::Display::None()).Build());
    }
    BoxEditor().AppendChild(list, std::move(line));
  }
  BoxTreeBuilder(list)
      .Begin<LineBox>(L"hover")
      .SetStyle(
          *css::StyleBuilder()
               .SetPosition(css::Position::Absolute())
               .SetLeft(css::Left(css::Length(0)))
               .SetTop(css::Top(css::Length(-1000)))
               .SetBackground(
                   css::Background(css::Color::Rgba(51, 153, 255, 0.1f)))
               .SetBorder(css::Border(css::Color::Rgba(51, 153, 255, 1.0f), 1))
               .Build())
      .Begin<TextBox>(L" ")
      .SetStyle(*kBlack)
      .End<TextBox>()
      .End<LineBox>()
      .Finish<BlockBox>(list);
  return std::move(root);
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
DemoModel::DemoModel() : root_box_(BuildBoxTree()) {}
DemoModel::~DemoModel() {}

void DemoModel::AttachWindow(DemoWindow* window) {
  window_ = window;
  RequestAnimationFrame();
}

LineBox* DemoModel::FindLineBox(const FloatPoint& point) const {
  Layouter().Layout(root_box_.get());
  const auto& found = BoxFinder(*root_box_).FindByPoint(point);
  if (!found.box)
    return nullptr;
  const auto list = root_box_->GetBoxById(L"list");
  const auto source = found.box;
  if (!source->IsDescendantOf(*list))
    return nullptr;
  for (const auto& runner : Box::AncestorsOrSelf(*source)) {
    if (const auto line = runner->as<LineBox>())
      return line;
  }
  return nullptr;
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
  const auto line = FindLineBox(point);
  if (!line)
    return;
  const auto hover = root_box_->GetBoxById(L"hover");
  const auto hover_point = line->bounds().origin();
  BoxEditor().SetStyle(hover,
                       *css::StyleBuilder()
                            .SetTop(css::Top(css::Length(hover_point.y())))
                            .SetLeft(css::Left(css::Length(hover_point.x())))
                            .Build());
  RequestAnimationFrame();
}

void DemoModel::DidPressMouse(const FloatPoint& point) {
  const auto line = FindLineBox(point);
  if (!line)
    return;
  const auto hover = root_box_->GetBoxById(L"hover");
  const auto hover_point = line->bounds().origin();
  BoxEditor().SetStyle(hover,
                       *css::StyleBuilder()
                            .SetTop(css::Top(css::Length(hover_point.y())))
                            .SetLeft(css::Left(css::Length(hover_point.x())))
                            .Build());
  RequestAnimationFrame();
}

const char* DemoModel::GetAnimationFrameType() const {
  return "DemoModel";
}

}  // namespace visuals
