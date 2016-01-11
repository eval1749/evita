// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "evita/visuals/demo/demo_model.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "evita/visuals/css/media_type.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/ancestors_or_self.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/geometry/float_rect.h"
#include "evita/visuals/layout/box_finder.h"
#include "evita/visuals/layout/box_tree.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree_builder.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"

namespace visuals {

namespace {

const auto kMargin = 8;
const auto kBorder = 1;

Document* LoadDocument() {
  const auto document =
      NodeTreeBuilder()
          .Begin(L"main")
          .SetInlineStyle(
              *css::StyleBuilder()
                   .SetBackground(css::Background(css::Color(1, 1, 1)))
                   .SetPadding(css::Padding(kMargin, kMargin, kMargin, 0))
                   .Build())
          .Begin(L"list", L"list")
          .End(L"list")
          .End(L"main")
          .Build();
  const auto list = document->GetElementById(L"list");
  for (auto index = 0; index < 20; ++index) {
    NodeTreeBuilder(list)
        .Begin(L"list_item")
        .Begin(L"name")
        .AddText(base::StringPrintf(L"name %d", index))
        .End(L"name")
        .Begin(L"size")
        .AddText(L"size")
        .End(L"size")
        .Begin(L"status")
        .AddText(L"status")
        .End(L"status")
        .Begin(L"file")
        .AddText(L"file")
        .End(L"file")
        .End(L"list_item")
        .Finish(list);
    if (index == 0) {
      NodeEditor().SetInlineStyle(
          list->last_child()->as<Element>(),
          *css::StyleBuilder().SetDisplay(css::Display::None()).Build());
    }
  }
  NodeTreeBuilder(list)
      .Begin(L"list_item", L"hover")
      .SetInlineStyle(*css::StyleBuilder()
                           .SetLeft(css::Left(css::Length(0)))
                           .SetTop(css::Top(css::Length(-1000)))
                           .Build())
      .AddText(L" ")
      .End(L"list_item")
      .Finish(list);
  return document;
}

css::StyleSheet* LoadStyleSheet() {
  const auto style_sheet = new css::StyleSheet();
  style_sheet->AddRule(
      L"#hover", std::move(css::StyleBuilder()
                               .SetPosition(css::Position::Absolute())
                               .SetLeft(css::Left(css::Length(20)))
                               .SetTop(css::Top(css::Length(300)))
                               .SetBackground(css::Background(
                                   css::Color::Rgba(51, 153, 255, 0.1f)))
                               .SetBorder(css::Border(
                                   css::Color::Rgba(51, 153, 255, 1.0f), 1))
                               .Build()));
  style_sheet->AddRule(
      L"list",
      std::move(css::StyleBuilder().SetDisplay(css::Display::Block()).Build()));
  style_sheet->AddRule(L"list_item",
                       std::move(css::StyleBuilder()
                                     .SetColor(css::Color(0, 0, 0))
                                     .SetDisplay(css::Display::Block())
                                     .SetPadding(css::Padding(2, 5, 2, 5))
                                     .Build()));
  style_sheet->AddRule(
      L"main",
      std::move(css::StyleBuilder().SetDisplay(css::Display::Block()).Build()));
  style_sheet->AddRule(L"name",
                       std::move(css::StyleBuilder().SetWidth(150).Build()));
  style_sheet->AddRule(
      L"size",
      std::move(css::StyleBuilder().SetMarginRight(css::Length(5)).Build()));
  style_sheet->AddRule(
      L"status",
      std::move(css::StyleBuilder().SetMarginRight(css::Length(5)).Build()));
  style_sheet->AddRule(
      L"file",
      std::move(css::StyleBuilder().SetMarginRight(css::Length(5)).Build()));
  return style_sheet;
}

void PrintBox(const BoxTree& box_tree) {
  std::cout << "Box Tree:" << std::endl << box_tree << std::endl;
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
DemoModel::DemoModel()
    : document_(LoadDocument()),
      style_sheet_(LoadStyleSheet()),
      style_tree_(new StyleTree(*document_, *this, {style_sheet_})),
      box_tree_(new BoxTree(*document_, *style_tree_)) {}

DemoModel::~DemoModel() {}

void DemoModel::AttachWindow(DemoWindow* window) {
  window_ = window;
  RequestAnimationFrame();
}

Element* DemoModel::FindListItem(const FloatPoint& point) {
  UpdateLayoutIfNeeded();
  const auto root_box = box_tree_->root_box();
  const auto& found = BoxFinder(*root_box).FindByPoint(point);
  if (!found.box)
    return nullptr;
  const auto list = document_->GetElementById(L"list");
  const auto source = found.box->node();
  if (!source->IsDescendantOf(*list))
    return nullptr;
  for (const auto& runner : Node::AncestorsOrSelf(*source)) {
    if (runner->node_name() == L"list_item")
      return runner->as<Element>();
  }
  return nullptr;
}

void DemoModel::UpdateLayoutIfNeeded() {
  UpdateStyleIfNeeded();
  box_tree_->UpdateIfNeeded();
  Layouter().Layout(box_tree_->root_box());
}

void DemoModel::UpdateStyleIfNeeded() {
  style_tree_->UpdateIfNeeded();
}

// css::Media
css::MediaType DemoModel::media_type() const {
  return css::MediaType::Screen;
}

FloatSize DemoModel::viewport_size() const {
  return viewport_size_;
}

// ui::AnimationFrameHandler
void DemoModel::DidBeginAnimationFrame(const base::TimeTicks& now) {
  const auto& canvas = window_->GetCanvas();
  if (!canvas)
    return RequestAnimationFrame();

  DCHECK(!viewport_size_.IsEmpty());

  UpdateStyleIfNeeded();
  UpdateLayoutIfNeeded();

  const auto root_box = box_tree_->root_box();
  if (root_box->IsPaintClean()) {
    // Box tree is changed outside viewport(?).
    return;
  }
  const auto& debug_text =
      base::StringPrintf(L"dom: %d, css: %d, box: %d", document_->version(),
                         style_tree_->version(), box_tree_->version());
  PaintInfo paint_info(root_box->bounds(), debug_text);
  auto display_item_list = Painter().Paint(paint_info, *root_box);

#if 0
  static base::Time last_time;
  if (now - last_time >= base::TimeDelta::FromMilliseconds(1000)) {
    last_time = now;
    PrintBox(*box_tree_);
    PrintPaint(*display_item_list);
  }
#endif

  DisplayItemListProcessor processor;
  processor.Paint(canvas, std::move(display_item_list));
}

// WindowEventHandler
void DemoModel::DidChangeWindowBounds(const FloatRect& bounds) {
  RequestAnimationFrame();
  viewport_size_ = bounds.size();
  css::Media::DidChangeViewportSize();
}

void DemoModel::DidMoveMouse(const FloatPoint& point) {
  const auto line = FindListItem(point);
  if (!line)
    return;
  const auto hover = document_->GetElementById(L"hover");
  const auto hover_point = box_tree_->BoxFor(*line)->bounds().origin();
  NodeEditor().SetInlineStyle(
      hover, *css::StyleBuilder()
                  .SetTop(css::Top(css::Length(hover_point.y())))
                  .SetLeft(css::Left(css::Length(hover_point.x())))
                  .Build());
  RequestAnimationFrame();
}

void DemoModel::DidPressMouse(const FloatPoint& point) {
  const auto line = FindListItem(point);
  if (!line)
    return;
  const auto hover = document_->GetElementById(L"hover");
  const auto hover_point = box_tree_->BoxFor(*line)->bounds().origin();
  NodeEditor().SetInlineStyle(
      hover, *css::StyleBuilder()
                  .SetTop(css::Top(css::Length(hover_point.y())))
                  .SetLeft(css::Left(css::Length(hover_point.x())))
                  .Build());
  PrintBox(*box_tree_);
  RequestAnimationFrame();
}

const char* DemoModel::GetAnimationFrameType() const {
  return "DemoModel";
}

}  // namespace visuals
