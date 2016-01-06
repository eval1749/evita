// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <unordered_map>
#include <vector>

#include "evita/visuals/layout/box_tree.h"

#include "base/logging.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_visitor.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/inline_flow_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/style/style_tree.h"

namespace visuals {

struct Context {
  bool is_update;
  const StyleTree* style_tree;
};

//////////////////////////////////////////////////////////////////////
//
// LayoutVisitor
//
class LayoutVisitor final : public NodeVisitor {
 public:
  LayoutVisitor(Context* context);
  ~LayoutVisitor() final;

  void Layout(const Node& node);

 private:
  const css::Style& ComputedStyleOf(const Node& node) const;
  std::unique_ptr<Box> GenerateBox(const Node& node);
  void ReturnBox(std::unique_ptr<Box> box);

// NodeVisitor
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_NODE(V)
#undef V

  Context* const context_;

  DISALLOW_COPY_AND_ASSIGN(LayoutVisitor);
};

LayoutVisitor::LayoutVisitor(Context* context) : context_(context) {}
LayoutVisitor::~LayoutVisitor() {}

const css::Style& LayoutVisitor::ComputedStyleOf(const Node& node) const {
  return context_->style_tree->ComputedStyleOf(node);
}

std::unique_ptr<Box> LayoutVisitor::GenerateBox(const Node& node) {
  DCHECK(!result_box_);
  Visit(node);
  box_map_->emplace(&node, result_box_.get());
  return std::move(result_box_);
}

void LayoutVisitor::ReturnBox(std::unique_ptr<Box> box) {
  DCHECK(!result_box_);
  result_box_ = std::move(box);
}

// NodeVisitor
void LayoutVisitor::VisitDocument(Document* document) {
  // TODO(eval1749): Load default style sheet
  for (const auto& child : document->child_nodes()) {
    if (auto const document_element = child->as<Element>()) {
      auto document_element_box = GenerateBox(*document_element);
      DCHECK(document_element_box) << "Oops, document element "
                                   << *document_element
                                   << " doesn't have a "
                                      "box. Maybe it has display:none: "
                                   << ComputedStyleOf(*document_element);
      BoxEditor().AppendChild(root_box_, document_element_box.release());
      return;
    }
  }
  NOTREACHED() << document << " is empty.";
}

// Full form of display-legacy
//  block           block flow
//  inline          inline flow
//  inline-block    inline flow-root
void LayoutVisitor::VisitElement(Element* element) {
  const auto& style = ComputedStyleOf(*element);
  if (style.display().is_none())
    return;
  std::vector<std::unique_ptr<Box>> child_boxes;
  std::vector<std::unique_ptr<Box>> inline_boxes;
  // Does "flow" formatting
  for (const auto& child : element->child_nodes()) {
    auto child_box = GenerateBox(*child);
    if (!child_box)
      continue;
    if (child_box->is<InlineBox>() || child_box->is<InlineFlowBox>()) {
      inline_boxes.push_back(std::move(child_box));
      continue;
    }
    const auto child_element = child->as<Element>();
    DCHECK(child_element) << child;
    const auto& child_style = ComputedStyleOf(*child_element);
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
          BoxEditor().AppendChild(inline_flow_box.get(), inline_box.release());
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
      BoxEditor().AppendChild(inline_flow_box.get(), inline_box.release());
    inline_boxes.clear();
    child_boxes.push_back(std::move(inline_flow_box));
  }
  if (style.display().is_block()) {
    auto block_flow_box = std::make_unique<BlockFlowBox>(root_box_, element);
    BoxEditor().SetStyle(block_flow_box.get(), style);
    for (auto& child_box : child_boxes)
      BoxEditor().AppendChild(block_flow_box.get(), child_box.release());
    return ReturnBox(std::move(block_flow_box));
  }
  auto inline_flow_box = std::make_unique<InlineFlowBox>(root_box_, element);
  BoxEditor().SetStyle(inline_flow_box.get(), style);
  for (auto& child_box : child_boxes)
    BoxEditor().AppendChild(inline_flow_box.get(), child_box.release());
  return ReturnBox(std::move(inline_flow_box));
}

void LayoutVisitor::VisitText(Text* text) {
  auto text_box = std::make_unique<TextBox>(root_box_, text->data(), text);
  const auto& style = ComputedStyleOf(*text);
  BoxEditor().SetTextColor(text_box.get(), style.color().value());
  ReturnBox(std::move(text_box));
}

//////////////////////////////////////////////////////////////////////
//
// BoxTree::Impl
//
class BoxTree::Impl final {
 public:
  Impl(const Document& document, const StyleTree& style_tree);
  ~Impl() = default;

  RootBox* root_box() const;
  int version() const { return version_; }

  Box* BoxFor(const Node& node) const;
  void MarkDirty(const Node& node);
  void UpdateIfNeeded();

 private:
  void AssignBoxToDocument(Context* context, const Document& document);
  void AssignBoxToElement(Context* context, const Element& element);
  void AssignBoxToText(Context* context, const Text& text);
  void UpdateContainerNode(Context* context, const ContainerNode& container);
  void UpdateContainerNodeIfNeeded(Context* context,
                                   const ContainerNode& container);
  void UpdateNode(Context* context, const Node& node);
  void UpdateNodeIfNeeded(Context* context, const Node& node);

  std::unordered_map<const Node*, Box*> box_map_;
  const Document& document_;
  std::unique_ptr<RootBox> root_box_;
  BoxTreeState state_;
  const StyleTree& style_tree_;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

BoxTree::Impl::Impl(const Document& document, const StyleTree& style_tree)
    : impl_(new Imple(document, style_tree) {
  root_box_(std::make_unique<RootBox>(document_)), state_(BoxTreeState::Dirty),
      style_tree_(style_tree) {
    RootBox* BoxTree::Impl::root_box() const {
      DCHECK_NE(BoxTreeState::Updating, state_);
      DCHECK_EQ(BoxTreeState::Clean, state_);
      return root_box_.get();
    }

    void BoxTree::Impl::AssignBoxToNode(Context * context,
                                        const Document& document) {
      for (const auto& child : document->child_nodes()) {
        if (auto const document_element = child->as<Element>()) {
          auto document_element_box = BoxOf(*document_element);
          DCHECK(document_element_box) << "Oops, document element "
                                       << *document_element
                                       << " doesn't have a "
                                          "box. Maybe it has display:none: "
                                       << ComputedStyleOf(*document_element);
          BoxEditor().AppendChild(root_box_, document_element_box);
          return;
        }
      }
      NOTREACHED() << document << " is empty.";
    }

    void BoxTree::Impl::AssignBoxToNode(Context * context, const Node& node) {
      if (const text = node.as<Text>())
        return AssignBoxToText(context, text);
      if (const element = node.as<Element>())
        return AssignBoxToElement(context, element);
      if (const document = node.as<Document>())
        return AssignBoxToDocument(context, document);
      NOTREACHED();
    }

    void BoxTree::Impl::AssignBoxToText(Context * context, const Text& text) {
      const auto it = box_map_.find(text);
      if (it == box_map_.end) {
        const auto& style = ComputedStyleOf(*text);
        auto new_text_box =
            std::make_unique<TextBox>(root_box_, text->data(), text);
        BoxEditor().SetTextColor(text_box.get(), style.color().value());
        box_map_.emplace(&text, std::move(new_text_box));
        return;
      }
      const auto text_box = it.second.get();
      const auto& style = ComputedStyleOf(*text);
      BoxEditor().SetTextColor(text_box.get(), style.color().value());
    }

    Box* BoxTree::Impl::BoxFor(const Node& node) const {
      DCHECK_EQ(BoxTreeState::Clean, state_);
      const auto& it = box_map_.find(&node);
      return it == box_map_.end() ? nullptr : it->second;
    }

    void BoxTree::Impl::MarkDirty(const Node& node) {
      if (state_ == BoxTreeState::Updating)
        return;
      state_ = BoxTreeState::Dirty;
      BoxEditor().MarkDirty(BoxFor(node));
    }

    void BoxTree::Impl::UpdateContainerNode(Context * context,
                                            ContainerNode & container) {
      for (const auto& child : container->child_nodes())
        UpdateNode(context, child);
      AssignBoxToNode(context, node);
    }

    void BoxTree::Impl::UpdateContainerNodeIfNeeded(Context * context,
                                                    ContainerNode & container) {
      const auto box = GetOrNewBox(container)->as<ContainerBox>();
      if (box->is_dirty())
        return UpdateContainerNode(context, container);
      if (!box->is_child_dirty())
        return;
      for (const auto& child : container->child_nodes())
        UpdateNodeIfNeeded(context, child);
      AssignBoxToNode(context, node);
    }

    void BoxTree::Impl::UpdateIfNeeded() {
      DCHECK_NE(BoxTreeState::Updating, state_);
      if (state_ == BoxTreeState::Clean) {
#if !DCHECK_IS_ON()
        for (const auto& node : Node::DescendantsOrSelf(document_)) {
          const auto box = BoxFor(node);
          DCHECK(!box->is_changed());
          const auto container = box->as<ContainerBox>();
          if (!container)
            continue;
          DCHECK(!container->is_child_changed());
        }
#endif
        return;
      }
      DCHECK(root_box_->is_changed() || root_box_->is_child_changed());
      BoxEditor().SetViewportSize(style_tree_.media().viewport_size());
      Context context;
      context.is_updated = false;
      state_ = BoxTreeState::Updating;
      if (root_box_->is_changed())
        UpdateContainerNode(&context, document_);
      else
        UpdateContainerNodeIfNeeded(&context, document_);
      DCHECK(context.is_update);
      state_ = BoxTreeState::Clean;
    }

    void BoxTree::Impl::UpdateNode(Context * context, Node & node) {
      if (const auto container = node.as<ContainerNode>())
        return UpdateContainerNode(container, container);
      AssignBoxToNode(context, node);
    }

    void BoxTree::Impl::UpdateNodeIfNeeded(Context * context, Node & node) {
      if (const auto container = node.as<ContainerNode>())
        return UpdateContainerNodeIfNeeded(container, container);
      const auto box = GetOrNewBox(container)->as<ContainerBox>();
      if (!box->is_dirty())
        return;
      AssignBoxToNode(context, node);
    }

    //////////////////////////////////////////////////////////////////////
    //
    // BoxTree
    //
BoxTree::BoxTree(const Document& document, const StyleTree& style_tree)
    : impl_(new Impl(document, style_tree) {
      document.AddObserver(this);
      style_tree.AddObserver(this);
}

BoxTree::~BoxTree() {
      impl_->document().RemoveObserver(this);
      impl_->style_tree().RemoveObserver(this);
}

RootBox* BoxTree::root_box() const {
      return impl_->root_box();
}

int BoxTree::version() const {
      return impl_->version();
}

Box* BoxTree::BoxFor(const Node& node) const {
      return impl_->BoxFor(node);
}

void BoxTree::UpdateIfNeeded() {
      return impl_->UpdateIfNeeded();
}

// DocumentObserver
void BoxTree::DidAppendChild(const ContainerNode& parent, const Node& child) {
      impl_->MarkDirty(parent);
}

void BoxTree::DidChangeInlineStyle(const Element& element,
                                   const css::Style* old_style) {
      // TODO(eval1749): We should not clear root box to optimize left/top
      // changes.
      impl_->MarkDirty(element);
}

void BoxTree::DidInsertBefore(const ContainerNode& parent,
                              const Node& child,
                              const Node& ref_child) {
      impl_->MarkDirty(parent);
}

void BoxTree::WillRemoveChild(const ContainerNode& parent, const Node& child) {
      impl_->MarkDirty(parent);
}

// StyleTreeObserver
void BoxTree::DidChangeComputedStyle(const Element& element,
                                     const css::Style& old_style) {
      impl_->MarkDirty(element);
}

  }  // namespace visuals
