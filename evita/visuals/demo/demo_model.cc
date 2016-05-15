// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "evita/visuals/demo/demo_model.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
// TODO(eval174): We should move |ui::KeyCode| to its own file from "event.h".
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/ui/events/event.h"
#include "evita/visuals/css/media_state.h"
#include "evita/visuals/css/media_type.h"
#include "evita/visuals/css/selector_parser.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/demo/demo_window.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/dom/ancestors_or_self.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/box_tree.h"
#include "evita/visuals/view/public/selection.h"
#include "evita/visuals/view/public/view_lifecycle.h"
#include "evita/visuals/view/view.h"

namespace visuals {

namespace {

const auto kMargin = 8;
const auto kBorder = 1;

css::Selector ParseSelector(base::StringPiece16 text) {
  return css::Selector::Parser().Parse(text);
}

Document* LoadDocument() {
  const auto document = NodeTreeBuilder()
                            .Begin(L"main")
                            .SetInlineStyle(*css::StyleBuilder()
                                                 .SetPaddingBottom(kMargin)
                                                 .SetPaddingRight(kMargin)
                                                 .SetPaddingTop(kMargin)
                                                 .Build())
                            .Begin(L"input", L"input")
                            .AddText(L"this is a text field.")
                            .End(L"input")
                            .AddShape({0})
                            .AddText(L"Check box 1")
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
          list->last_child()->as<ElementNode>(),
          *css::StyleBuilder().SetDisplay(css::Display::None()).Build());
    }
  }
  NodeTreeBuilder(list)
      .Begin(L"list_item", L"hover")
      .SetInlineStyle(*css::StyleBuilder().SetLeft(0).SetTop(-1000).Build())
      .AddText(L" ")
      .End(L"list_item")
      .Finish(list);
  return document;
}

css::StyleSheet* LoadStyleSheet() {
  const auto style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      ParseSelector(L"#hover"),
      std::move(
          css::StyleBuilder()
              .SetPosition(css::Position::Absolute())
              .SetLeft(20)
              .SetTop(300)
              .SetBackgroundColor(css::ColorValue::Rgba(51, 153, 255, 0.1f))
              .SetBorder(css::ColorValue::Rgba(51, 153, 255, 1.0f), 1)
              .Build()));
  style_sheet->AppendRule(
      ParseSelector(L"input"),
      std::move(css::StyleBuilder()
                    .SetBorder(css::ColorValue::Rgba(128, 128, 128), 1)
                    .SetPadding(2)
                    .SetWidth(300)
                    .Build()));
  style_sheet->AppendRule(
      ParseSelector(L"list"),
      std::move(css::StyleBuilder().SetDisplay(css::Display::Block()).Build()));
  style_sheet->AppendRule(ParseSelector(L"list_item"),
                          std::move(css::StyleBuilder()
                                        .SetColor(0, 0, 0)
                                        .SetDisplay(css::Display::Block())
                                        .SetPaddingTop(2)
                                        .SetPaddingRight(5)
                                        .SetPaddingBottom(2)
                                        .SetPaddingLeft(5)
                                        .Build()));
  style_sheet->AppendRule(
      ParseSelector(L"main"),
      std::move(css::StyleBuilder().SetDisplay(css::Display::Block()).Build()));
  style_sheet->AppendRule(ParseSelector(L"name"),
                          std::move(css::StyleBuilder().SetWidth(150).Build()));
  style_sheet->AppendRule(
      ParseSelector(L"size"),
      std::move(css::StyleBuilder().SetMarginRight(5).Build()));
  style_sheet->AppendRule(
      ParseSelector(L"status"),
      std::move(css::StyleBuilder().SetMarginRight(5).Build()));
  style_sheet->AppendRule(
      ParseSelector(L"file"),
      std::move(css::StyleBuilder().SetMarginRight(5).Build()));
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
      view_(new View(*document_, *this, {style_sheet_})) {
  view_->AddObserver(this);
}

DemoModel::~DemoModel() {
  view_->RemoveObserver(this);
}

Selection* DemoModel::selection() {
  return view_->selection();
}

void DemoModel::AttachWindow(DemoWindow* window) {
  window_ = window;
  selection()->Collapse(document_->GetElementById(L"input")->first_child(), 1);
}

ElementNode* DemoModel::FindListItem(const gfx::FloatPoint& point) {
  const auto& found = view_->HitTest(point);
  if (!found)
    return nullptr;
  const auto list = document_->GetElementById(L"list");
  const auto source = found.node();
  if (!source->IsDescendantOf(*list))
    return nullptr;
  for (const auto& runner : Node::AncestorsOrSelf(*source)) {
    if (runner->node_name() == L"list_item")
      return runner->as<ElementNode>();
  }
  return nullptr;
}

// css::Media
css::MediaState DemoModel::media_state() const {
  // TODO(eval1749): Should we consider popup window?
  return window_->has_focus() ? css::MediaState::Interactive
                              : css::MediaState::Inactive;
}

css::MediaType DemoModel::media_type() const {
  return css::MediaType::Screen;
}

gfx::FloatSize DemoModel::viewport_size() const {
  return viewport_size_;
}

// ui::AnimationFrameHandler
void DemoModel::DidBeginAnimationFrame(const base::TimeTicks& now) {
  const auto& canvas = window_->GetCanvas();
  if (!canvas)
    return RequestAnimationFrame();

  DCHECK(!viewport_size_.IsEmpty());

  auto display_item_list = view_->Paint();
  if (!display_item_list)
    return;

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

const char* DemoModel::GetAnimationFrameType() const {
  return "DemoModel";
}

// ViewObserver
void DemoModel::DidChangeView() {
  RequestAnimationFrame();
}

// WindowEventHandler
void DemoModel::DidChangeWindowBounds(const gfx::FloatRect& bounds) {
  viewport_size_ = bounds.size();
  css::Media::DidChangeViewportSize();
}

void DemoModel::DidKillFocus() {
  DidChangeMediaState();
}

void DemoModel::DidMoveMouse(const gfx::FloatPoint& point) {
  const auto line = FindListItem(point);
  if (!line)
    return;
  const auto hover = document_->GetElementById(L"hover");
  const auto hover_point = view_->ComputeBorderBoxQuad(*line).point1();
  NodeEditor().SetInlineStyle(hover, *css::StyleBuilder()
                                          .SetTop(hover_point.y())
                                          .SetLeft(hover_point.x())
                                          .Build());
}

void DemoModel::DidPressKey(int key_code) {
  const auto node = const_cast<Text*>(selection()->focus_node().as<Text>());
  const auto offset = selection()->focus_offset();
  switch (key_code) {
    case ui::KeyCode::ArrowLeft:
      if (offset > 0)
        selection()->Collapse(node, offset - 1);
      break;
    case ui::KeyCode::ArrowRight:
      if (offset < node->data().size()) {
        selection()->Collapse(node, offset + 1);
      }
      break;
    case ui::KeyCode::ShiftArrowLeft:
      if (offset > 0)
        selection()->ExtendTo(node, offset - 1);
      break;
    case ui::KeyCode::ShiftArrowRight:
      if (offset < node->data().size()) {
        selection()->ExtendTo(node, offset + 1);
      }
      break;
    default:
      std::cout << "DidPressKey key_code=" << std::hex << key_code << std::endl;
      break;
  }
}

void DemoModel::DidPressMouse(const gfx::FloatPoint& point) {
  const auto line = FindListItem(point);
  if (!line)
    return;
  const auto hover = document_->GetElementById(L"hover");
  const auto hover_point = view_->ComputeBorderBoxQuad(*line).point1();
  NodeEditor().SetInlineStyle(hover, *css::StyleBuilder()
                                          .SetTop(hover_point.y())
                                          .SetLeft(hover_point.x())
                                          .Build());
  PrintBox(view_->box_tree());
}

void DemoModel::DidSetFocus() {
  DidChangeMediaState();
}

}  // namespace visuals
