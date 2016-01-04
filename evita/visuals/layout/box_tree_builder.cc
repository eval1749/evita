// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <vector>

#include "evita/visuals/layout/box_tree_builder.h"

#include "base/logging.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_visitor.h"
#include "evita/visuals/dom/text_node.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/inline_flow_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/style/style_resolver.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// GenerateVisitor
//
class GenerateVisitor final : public NodeVisitor {
 public:
  GenerateVisitor(RootBox* root_box,
                  StyleResolver* style_resolver,
                  std::unordered_map<const Node*, Box*>* box_map);
  ~GenerateVisitor() final;

  void BuildAll();

 private:
  std::unique_ptr<Box> GenerateBox(const Node& node);
  const css::Style& ResolveStyleFor(const Node& node) const;
  void ReturnBox(std::unique_ptr<Box> box);

// NodeVisitor
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_NODE(V)
#undef V

  std::unordered_map<const Node*, Box*>* const box_map_;
  std::unique_ptr<Box> result_box_;
  RootBox* const root_box_;
  StyleResolver* const style_resolver_;

  DISALLOW_COPY_AND_ASSIGN(GenerateVisitor);
};

GenerateVisitor::GenerateVisitor(RootBox* root_box,
                                 StyleResolver* style_resolver,
                                 std::unordered_map<const Node*, Box*>* box_map)
    : box_map_(box_map), root_box_(root_box), style_resolver_(style_resolver) {}

GenerateVisitor::~GenerateVisitor() {}

void GenerateVisitor::BuildAll() {
  Visit(style_resolver_->document());
}

std::unique_ptr<Box> GenerateVisitor::GenerateBox(const Node& node) {
  DCHECK(!result_box_);
  Visit(node);
  box_map_->emplace(&node, result_box_.get());
  return std::move(result_box_);
}

const css::Style& GenerateVisitor::ResolveStyleFor(const Node& node) const {
  return style_resolver_->ResolveFor(node);
}

void GenerateVisitor::ReturnBox(std::unique_ptr<Box> box) {
  DCHECK(!result_box_);
  result_box_ = std::move(box);
}

// NodeVisitor
void GenerateVisitor::VisitDocument(Document* document) {
  // TODO(eval1749): Load default style sheet
  for (const auto& child : document->child_nodes()) {
    if (auto const element = child->as<Element>())
      BoxEditor().AppendChild(root_box_, std::move(GenerateBox(*element)));
    return;
  }
  NOTREACHED() << document << " is empty.";
}

// Full form of display-legacy
//  block           block flow
//  inline          inline flow
//  inline-block    inline flow-root
void GenerateVisitor::VisitElement(Element* element) {
  const auto& style = ResolveStyleFor(*element);
  if (style.display().is_none())
    return;
  std::vector<std::unique_ptr<Box>> child_boxes;
  std::vector<std::unique_ptr<Box>> inline_boxes;
  // Does "flow" formatting
  for (const auto& child : element->child_nodes()) {
    auto child_box = GenerateBox(*child);
    if (child_box->is<InlineBox>() || child_box->is<InlineFlowBox>()) {
      inline_boxes.push_back(std::move(child_box));
      continue;
    }
    const auto child_element = child->as<Element>();
    DCHECK(child_element) << child;
    const auto& child_style = ResolveStyleFor(*child_element);
    if (child_style.display().is_inline() ||
        child_style.display().is_inline_block()) {
      inline_boxes.push_back(std::move(child_box));
      continue;
    }
    switch (inline_boxes.size()) {
      case 0:
        break;
      case 1:
        child_boxes.push_back(std::move(inline_boxes.front()));
        inline_boxes.pop_back();
        break;
      default: {
        auto inline_flow_box = std::make_unique<InlineFlowBox>(root_box_);
        for (auto& inline_box : inline_boxes)
          BoxEditor().AppendChild(inline_flow_box.get(), std::move(inline_box));
        inline_boxes.clear();
        child_boxes.push_back(std::move(inline_flow_box));
        break;
      }
    }
    child_boxes.push_back(std::move(child_box));
  }
  if (inline_boxes.size() == 1) {
    child_boxes.push_back(std::move(inline_boxes.front()));
  } else if (!inline_boxes.empty()) {
    auto inline_flow_box = std::make_unique<InlineFlowBox>(root_box_);
    for (auto& inline_box : inline_boxes)
      BoxEditor().AppendChild(inline_flow_box.get(), std::move(inline_box));
    inline_boxes.clear();
    child_boxes.push_back(std::move(inline_flow_box));
  }
  if (style.display().is_block()) {
    auto block_flow_box = std::make_unique<BlockFlowBox>(root_box_, element);
    BoxEditor().SetStyle(block_flow_box.get(), style);
    for (auto& child_box : child_boxes)
      BoxEditor().AppendChild(block_flow_box.get(), std::move(child_box));
    return ReturnBox(std::move(block_flow_box));
  }
  auto inline_box = std::make_unique<InlineFlowBox>(root_box_, element);
  BoxEditor().SetStyle(inline_box.get(), style);
  for (auto& child_box : child_boxes)
    BoxEditor().AppendChild(inline_box.get(), std::move(child_box));
  return ReturnBox(std::move(inline_box));
}

void GenerateVisitor::VisitTextNode(TextNode* text_node) {
  auto text_box =
      std::make_unique<TextBox>(root_box_, text_node->text(), text_node);
  const auto& style = ResolveStyleFor(*text_node);
  BoxEditor().SetTextColor(text_box.get(), style.color().value());
  ReturnBox(std::move(text_box));
}

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder
//
BoxTreeBuilder::BoxTreeBuilder(const Document& document,
                               const css::Media& media)
    : document_(document), style_resolver_(new StyleResolver(document, media)) {
  style_resolver_->AddObserver(this);
}

BoxTreeBuilder::~BoxTreeBuilder() {
  style_resolver_->RemoveObserver(this);
}

RootBox* BoxTreeBuilder::Build() {
  // TODO(eval1749): We should update box tree rather than build always.
  if (root_box_)
    return root_box_.get();
  DCHECK(box_map_.empty());
  root_box_ = std::make_unique<RootBox>(document_);
  box_map_.emplace(&document_, root_box_.get());
  GenerateVisitor(root_box_.get(), style_resolver_.get(), &box_map_).BuildAll();
  return root_box_.get();
}

void BoxTreeBuilder::Clear() {
  box_map_.clear();
  root_box_.reset();
}

// DocumentObserver
void BoxTreeBuilder::DidAppendChild(const ContainerNode& parent,
                                    const Node& child) {
  Clear();
}

void BoxTreeBuilder::DidChangeInlineStyle(const Element& element,
                                          const css::Style* old_style) {}
void BoxTreeBuilder::DidInsertBefore(const ContainerNode& parent,
                                     const Node& child,
                                     const Node& ref_child) {
  Clear();
}

void BoxTreeBuilder::WillRemoveChild(const ContainerNode& parent,
                                     const Node& child) {
  Clear();
}

// StyleChangeObserver
void BoxTreeBuilder::DidClearStyleCache() {
  Clear();
}

void BoxTreeBuilder::DidRemoveStyleCache(const Element& element,
                                         const css::Style& old_style) {
  Clear();
}

}  // namespace visuals
