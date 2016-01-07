// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "evita/visuals/layout/box_tree.h"

#include "base/logging.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_visitor.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/block_flow_box.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/inline_box.h"
#include "evita/visuals/layout/inline_flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

namespace {

// TODO(eval1749) We should move |IsDisplayOutsideInline()| to |css::Display|.
bool IsDisplayOutsideInline(const css::Display& display) {
  return display.is_inline() || display.is_inline_block();
}

//////////////////////////////////////////////////////////////////////
//
// BoxTreeState
//
#define FOR_EACH_BOX_TREE_STATE(V) \
  V(Clean)                         \
  V(Dirty)                         \
  V(Updating)

enum class BoxTreeState {
#define V(name) name,
  FOR_EACH_BOX_TREE_STATE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, BoxTreeState state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_BOX_TREE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

//////////////////////////////////////////////////////////////////////
//
// Context
//
struct Context {
  bool is_updated;
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BoxTree::Impl
//
class BoxTree::Impl final : public DocumentObserver, public StyleTreeObserver {
 public:
  Impl(const Document& document, const StyleTree& style_tree);
  ~Impl() final;

  RootBox* root_box() const;
  int version() const { return version_; }

  Box* BoxFor(const Node& node) const;
  void MarkDirty(const Node& node);
  void UpdateIfNeeded();

 private:
  Box* AssignBoxToElement(Context* context, const Element& element);
  Box* AssignBoxToNode(Context* context, const Node& node);
  Box* AssignBoxToText(Context* context, const Text& text);
  const css::Style& ComputedStyleOf(const Node& node) const;
  InlineFlowBox* CreateAnonymousInlineFlowBox(
      const std::vector<Box*>& children);
  void FormatContainerBox(Context* context, ContainerBox* container_box);
  void FormatBlockFlowBox(Context* context, ContainerBox* container_box);
  void FormatRootBox(Context* context, RootBox* root_box);
  void FormatInlineFlowBox(Context* context, InlineBox* inline_box);
  void IncrementVersionIfNeeded(Context* context);
  Box* RegisterBoxFor(Context* context,
                      const Node& node,
                      std::unique_ptr<Box> box);

  void RemoveAllChildren(ContainerBox* container_box);
  void UpdateContainerNode(Context* context, const ContainerNode& container);
  void UpdateContainerNodeIfNeeded(Context* context,
                                   const ContainerNode& container);
  void UpdateNode(Context* context, const Node& node);
  void UpdateNodeIfNeeded(Context* context, const Node& node);

  // DocumentObserver
  void DidAppendChild(const ContainerNode& parent, const Node& child) final;
  void DidChangeInlineStyle(const Element& element,
                            const css::Style* old_style) final;
  void DidInsertBefore(const ContainerNode& parent,
                       const Node& child,
                       const Node& ref_child) final;
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  // StyleTreeObserver
  void DidChangeComputedStyle(const Element& element,
                              const css::Style& old_style) final;

  std::unordered_map<const Node*, std::unique_ptr<Box>> box_map_;
  const Document& document_;
  RootBox* const root_box_;
  BoxTreeState state_;
  const StyleTree& style_tree_;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

BoxTree::Impl::Impl(const Document& document, const StyleTree& style_tree)
    : document_(document),
      root_box_(new RootBox(document_)),
      state_(BoxTreeState::Dirty),
      style_tree_(style_tree) {
  box_map_.emplace(&document_, std::unique_ptr<Box>(root_box_));
  document_.AddObserver(this);
  style_tree_.AddObserver(this);
}

BoxTree::Impl::~Impl() {
  document_.RemoveObserver(this);
  style_tree_.RemoveObserver(this);
  BoxEditor().RemoveDescendants(root_box_);
}

RootBox* BoxTree::Impl::root_box() const {
  DCHECK_NE(BoxTreeState::Updating, state_);
  DCHECK_EQ(BoxTreeState::Clean, state_);
  return root_box_;
}

Box* BoxTree::Impl::AssignBoxToElement(Context* context,
                                       const Element& element) {
  const auto& style = ComputedStyleOf(element);
  const auto& it = box_map_.find(&element);
  if (style.display().is_none()) {
    if (it == box_map_.end())
      return nullptr;
    IncrementVersionIfNeeded(context);
    RemoveAllChildren(it->second->as<ContainerBox>());
    box_map_.erase(it);
    return nullptr;
  }
  if (it != box_map_.end()) {
    const auto container_box = it->second->as<ContainerBox>();
    RemoveAllChildren(container_box);
    if (container_box->display() == style.display()) {
      BoxEditor().SetStyle(container_box, style);
      return container_box;
    }
    box_map_.erase(it);
  }
  if (style.display().is_block()) {
    // TODO(eval1749): |Box| constructor should take |css::Style|.
    auto new_block_flow_box =
        std::make_unique<BlockFlowBox>(root_box_, &element);
    BoxEditor().SetStyle(new_block_flow_box.get(), style);
    return RegisterBoxFor(context, element, std::move(new_block_flow_box));
  }
  if (style.display().is_inline()) {
    auto new_inline_box = std::make_unique<InlineBox>(root_box_, &element);
    BoxEditor().SetStyle(new_inline_box.get(), style);
    return RegisterBoxFor(context, element, std::move(new_inline_box));
  }
  if (style.display().is_inline_block()) {
    auto new_inline_flow_box =
        std::make_unique<InlineFlowBox>(root_box_, &element);
    BoxEditor().SetStyle(new_inline_flow_box.get(), style);
    return RegisterBoxFor(context, element, std::move(new_inline_flow_box));
  }
  NOTREACHED() << "Unsupported display:" << style.display() << " of "
               << element;
  return nullptr;
}

Box* BoxTree::Impl::AssignBoxToNode(Context* context, const Node& node) {
  IncrementVersionIfNeeded(context);
  if (const auto text = node.as<Text>())
    return AssignBoxToText(context, *text);
  if (const auto element = node.as<Element>())
    return AssignBoxToElement(context, *element);
  if (&node == &document_)
    return root_box_;
  NOTREACHED();
  return nullptr;
}

Box* BoxTree::Impl::AssignBoxToText(Context* context, const Text& text) {
  const auto& style = ComputedStyleOf(text);
  if (const auto text_box = BoxFor(text)) {
    BoxEditor().SetStyle(text_box, style);
    return text_box;
  }
  // TODO(eval1749): We should make |TextBox| constructor to take color.
  // TODO(eval1749): |TextBox| constructor should take |const Text&|.
  auto new_text_box = std::make_unique<TextBox>(root_box_, text.data(), &text);
  BoxEditor().SetStyle(new_text_box.get(), style);
  return RegisterBoxFor(context, text, std::move(new_text_box));
}

Box* BoxTree::Impl::BoxFor(const Node& node) const {
  DCHECK_NE(BoxTreeState::Dirty, state_);
  const auto& it = box_map_.find(&node);
  return it == box_map_.end() ? nullptr : it->second.get();
}

const css::Style& BoxTree::Impl::ComputedStyleOf(const Node& node) const {
  return style_tree_.ComputedStyleOf(node);
}

InlineFlowBox* BoxTree::Impl::CreateAnonymousInlineFlowBox(
    const std::vector<Box*>& children) {
  const auto inline_flow_box = new InlineFlowBox(root_box_);
  for (const auto& child : children)
    BoxEditor().AppendChild(inline_flow_box, child);
  return inline_flow_box;
}

// Full form of display-legacy
//  block           block flow
//  inline          inline flow
//  inline-block    inline flow-root
void BoxTree::Impl::FormatContainerBox(Context* context,
                                       ContainerBox* container_box) {
  if (auto const root_box = container_box->as<RootBox>())
    return FormatRootBox(context, root_box);
  if (auto const inline_box = container_box->as<InlineBox>())
    return FormatInlineFlowBox(context, inline_box);
  FormatBlockFlowBox(context, container_box);
}

void BoxTree::Impl::FormatBlockFlowBox(Context* context,
                                       ContainerBox* container_box) {
  DCHECK(container_box->node()->is<Element>());
  std::vector<Box*> child_boxes;
  std::vector<Box*> inline_boxes;
  // Does "flow" formatting
  for (const auto& child :
       container_box->node()->as<ContainerNode>()->child_nodes()) {
    const auto child_box = BoxFor(*child);
    if (!child_box) {
      DCHECK(ComputedStyleOf(*child).display().is_none()) << "No box for "
                                                          << *child;
      continue;
    }
    if (IsDisplayOutsideInline(child_box->display())) {
      inline_boxes.push_back(child_box);
      continue;
    }
    if (!inline_boxes.empty()) {
      child_boxes.push_back(CreateAnonymousInlineFlowBox(inline_boxes));
      inline_boxes.clear();
    }
    child_boxes.push_back(child_box);
  }

  if (!inline_boxes.empty()) {
    child_boxes.push_back(CreateAnonymousInlineFlowBox(inline_boxes));
    inline_boxes.clear();
  }

  for (auto& child_box : child_boxes)
    BoxEditor().AppendChild(container_box, child_box);
}

// Does "flow" formatting for inline-level element
void BoxTree::Impl::FormatInlineFlowBox(Context* context,
                                        InlineBox* inline_box) {
  DCHECK(inline_box->node()->is<Element>());
  for (const auto& child :
       inline_box->node()->as<ContainerNode>()->child_nodes()) {
    const auto child_box = BoxFor(*child);
    if (!child_box) {
      DCHECK(child_box->display().is_none()) << "No box for " << *child;
      continue;
    }
    if (IsDisplayOutsideInline(child_box->display())) {
      BoxEditor().AppendChild(inline_box, child_box);
      continue;
    }
    BoxEditor().AppendChild(inline_box,
                            CreateAnonymousInlineFlowBox({child_box}));
  }
}

void BoxTree::Impl::FormatRootBox(Context* context, RootBox* root_box) {
  BoxEditor().RemoveAllChildren(root_box_);
  for (const auto& child : document_.child_nodes()) {
    if (auto const document_element = child->as<Element>()) {
      auto document_element_box = BoxFor(*document_element);
      DCHECK(document_element_box) << "Oops, document element "
                                   << *document_element
                                   << " doesn't have a "
                                      "box. Maybe it has display:none: "
                                   << ComputedStyleOf(*document_element);
      BoxEditor().AppendChild(root_box_, document_element_box);
      return;
    }
  }
  NOTREACHED() << document_ << " is empty.";
}

void BoxTree::Impl::IncrementVersionIfNeeded(Context* context) {
  if (context->is_updated)
    return;
  context->is_updated = true;
  ++version_;
}

void BoxTree::Impl::MarkDirty(const Node& node) {
  if (state_ == BoxTreeState::Updating)
    return;
  state_ = BoxTreeState::Dirty;
  const auto& it = box_map_.find(&node);
  if (it == box_map_.end())
    return;
  BoxEditor().MarkDirty(it->second.get());
}

Box* BoxTree::Impl::RegisterBoxFor(Context* context,
                                   const Node& node,
                                   std::unique_ptr<Box> box) {
  IncrementVersionIfNeeded(context);
  const auto& result = box_map_.emplace(&node, std::move(box));
  DCHECK(result.second) << node << " is already in box_map_";
  return result.first->second.get();
}

void BoxTree::Impl::RemoveAllChildren(ContainerBox* container) {
  while (const auto child = container->first_child()) {
    BoxEditor().RemoveChild(container, child);
    if (!child->node()) {
      // Delete anonymous box.
      if (const auto anonymous_container = child->as<ContainerBox>())
        RemoveAllChildren(anonymous_container);
      delete child;
    }
  }
}

void BoxTree::Impl::UpdateContainerNode(Context* context,
                                        const ContainerNode& container) {
  AssignBoxToNode(context, container);
  const auto container_box = BoxFor(container);
  if (!container_box)
    return;
  for (const auto& child : container.child_nodes())
    UpdateNode(context, *child);
  FormatContainerBox(context, container_box->as<ContainerBox>());
}

void BoxTree::Impl::UpdateContainerNodeIfNeeded(
    Context* context,
    const ContainerNode& container) {
  if (const auto box = BoxFor(container)) {
    if (box->is_changed())
      return UpdateContainerNode(context, container);
    if (!box->as<ContainerBox>()->is_child_changed())
      return;
  }
  AssignBoxToNode(context, container);
  const auto container_box = BoxFor(container);
  if (!container_box)
    return;
  for (const auto& child : container.child_nodes())
    UpdateNodeIfNeeded(context, *child);
  FormatContainerBox(context, container_box->as<ContainerBox>());
}

void BoxTree::Impl::UpdateIfNeeded() {
  DCHECK_NE(BoxTreeState::Updating, state_);
  if (state_ == BoxTreeState::Clean) {
#if !DCHECK_IS_ON()
    DCHECK_EQ(style_tree_.media().viewport_size(), root_box_->viewport_size());
    for (const auto& node : Node::DescendantsOrSelf(document_)) {
      const auto box = BoxFor(*node);
      DCHECK(!box->is_changed());
      const auto container_box = box->as<ContainerBox>();
      if (!container_box)
        continue;
      DCHECK(!container_box->is_child_changed());
    }
#endif
    return;
  }
  DCHECK(root_box_->is_changed() || root_box_->is_child_changed());
  BoxEditor().SetViewportSize(root_box_, style_tree_.media().viewport_size());
  Context context;
  context.is_updated = false;
  state_ = BoxTreeState::Updating;
  if (root_box_->is_changed())
    UpdateContainerNode(&context, document_);
  else
    UpdateContainerNodeIfNeeded(&context, document_);
  DCHECK(context.is_updated);
  state_ = BoxTreeState::Clean;
}

void BoxTree::Impl::UpdateNode(Context* context, const Node& node) {
  if (const auto container = node.as<ContainerNode>())
    return UpdateContainerNode(context, *container);
  AssignBoxToNode(context, node);
}

void BoxTree::Impl::UpdateNodeIfNeeded(Context* context, const Node& node) {
  if (const auto container = node.as<ContainerNode>())
    return UpdateContainerNodeIfNeeded(context, *container);
  if (const auto box = BoxFor(node)) {
    if (!box->is_changed())
      return;
  }
  AssignBoxToNode(context, node);
}

// DocumentObserver
void BoxTree::Impl::DidAppendChild(const ContainerNode& parent,
                                   const Node& child) {
  MarkDirty(parent);
}

void BoxTree::Impl::DidChangeInlineStyle(const Element& element,
                                         const css::Style* old_style) {
  // TODO(eval1749): We should not clear root box to optimize left/top
  // changes.
  MarkDirty(element);
}

void BoxTree::Impl::DidInsertBefore(const ContainerNode& parent,
                                    const Node& child,
                                    const Node& ref_child) {
  MarkDirty(parent);
}

void BoxTree::Impl::WillRemoveChild(const ContainerNode& parent,
                                    const Node& child) {
  MarkDirty(parent);
}

// StyleTreeObserver
void BoxTree::Impl::DidChangeComputedStyle(const Element& element,
                                           const css::Style& old_style) {
  MarkDirty(element);
}

//////////////////////////////////////////////////////////////////////
//
// BoxTree
//
BoxTree::BoxTree(const Document& document, const StyleTree& style_tree)
    : impl_(new Impl(document, style_tree)) {}

BoxTree::~BoxTree() {}

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

}  // namespace visuals
