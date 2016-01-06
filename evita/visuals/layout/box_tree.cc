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
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_visitor.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/inline_flow_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

namespace {

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

// TODO(eval1749): We should have |Box::display()|
css::Display DisplayOf(const Box& box) {
  if (box.is<BlockFlowBox>())
    return css::Display::Block();
  if (box.is<InlineFlowBox>())
    return css::Display::InlineBlock();
  return css::Display::Inline();
}

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
  void AssignBoxToDocument(Context* context, const Document& document);
  void AssignBoxToElement(Context* context, const Element& element);
  void AssignBoxToNode(Context* context, const Node& node);
  void AssignBoxToText(Context* context, const Text& text);
  const css::Style& ComputedStyleOf(const Node& node) const;
  InlineFlowBox* CreateAnonymousBox(const std::vector<Box*>& children);
  void FormatFlow(Context* context, const Element& element);
  void IncrementVersionIfNeeded(Context* context);
  void RegisterBoxFor(Context* context,
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

void BoxTree::Impl::AssignBoxToDocument(Context* context,
                                        const Document& document) {
  BoxEditor().RemoveAllChildren(root_box_);
  for (const auto& child : document.child_nodes()) {
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
  NOTREACHED() << document << " is empty.";
}

void BoxTree::Impl::AssignBoxToElement(Context* context,
                                       const Element& element) {
  const auto& style = ComputedStyleOf(element);
  const auto& it = box_map_.find(&element);
  if (style.display().is_none()) {
    if (it == box_map_.end())
      return;
    IncrementVersionIfNeeded(context);
    RemoveAllChildren(it->second->as<ContainerBox>());
    box_map_.erase(it);
    return;
  }
  if (it != box_map_.end()) {
    const auto container_box = it->second->as<ContainerBox>();
    RemoveAllChildren(container_box);
    if (DisplayOf(*container_box) == style.display()) {
      BoxEditor().SetStyle(container_box, style);
      return FormatFlow(context, element);
    }
  }
  if (style.display().is_block()) {
    // TODO(eval1749): |Box| constructor should take |css::Style|.
    auto new_block_flow_box =
        std::make_unique<BlockFlowBox>(root_box_, &element);
    BoxEditor().SetStyle(new_block_flow_box.get(), style);
    RegisterBoxFor(context, element, std::move(new_block_flow_box));
    return FormatFlow(context, element);
  }
  if (style.display().is_inline_block() || style.display().is_inline()) {
    auto new_inline_box = std::make_unique<InlineFlowBox>(root_box_, &element);
    BoxEditor().SetStyle(new_inline_box.get(), style);
    RegisterBoxFor(context, element, std::move(new_inline_box));
    return FormatFlow(context, element);
  }
  NOTREACHED() << "Unsupported display:" << style.display() << " of "
               << element;
}

void BoxTree::Impl::AssignBoxToNode(Context* context, const Node& node) {
  if (const auto text = node.as<Text>())
    return AssignBoxToText(context, *text);
  if (const auto element = node.as<Element>())
    return AssignBoxToElement(context, *element);
  if (&node == &document_)
    return AssignBoxToDocument(context, document_);
  NOTREACHED();
}

void BoxTree::Impl::AssignBoxToText(Context* context, const Text& text) {
  const auto& style = ComputedStyleOf(text);
  if (const auto text_box = BoxFor(text))
    return BoxEditor().SetStyle(text_box, style);
  // TODO(eval1749): We should make |TextBox| constructor to take color.
  // TODO(eval1749): |TextBox| constructor should take |const Text&|.
  auto new_text_box = std::make_unique<TextBox>(root_box_, text.data(), &text);
  BoxEditor().SetStyle(new_text_box.get(), style);
  RegisterBoxFor(context, text, std::move(new_text_box));
}

Box* BoxTree::Impl::BoxFor(const Node& node) const {
  DCHECK_NE(BoxTreeState::Dirty, state_);
  const auto& it = box_map_.find(&node);
  return it == box_map_.end() ? nullptr : it->second.get();
}

const css::Style& BoxTree::Impl::ComputedStyleOf(const Node& node) const {
  return style_tree_.ComputedStyleOf(node);
}

InlineFlowBox* BoxTree::Impl::CreateAnonymousBox(
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
void BoxTree::Impl::FormatFlow(Context* context, const Element& element) {
  std::vector<Box*> child_boxes;
  std::vector<Box*> inline_boxes;
  // Does "flow" formatting
  for (const auto& child : element.child_nodes()) {
    const auto child_box = BoxFor(*child);
    if (!child_box) {
      DCHECK(ComputedStyleOf(*child).display().is_none()) << "No box for "
                                                          << *child;
      continue;
    }
    if (child_box->is<InlineBox>() || child_box->is<InlineFlowBox>()) {
      inline_boxes.push_back(child_box);
      continue;
    }
    const auto child_element = child->as<Element>();
    DCHECK(child_element) << child;
    const auto& child_style = ComputedStyleOf(*child_element);
    if (child_style.display().is_inline() ||
        child_style.display().is_inline_block()) {
      inline_boxes.push_back(child_box);
      continue;
    }
    switch (inline_boxes.size()) {
      case 0:
        break;
      case 1:
        child_boxes.push_back(inline_boxes.front());
        inline_boxes.pop_back();
        break;
      default: {
        child_boxes.push_back(CreateAnonymousBox(inline_boxes));
        inline_boxes.clear();
        break;
      }
    }
    child_boxes.push_back(child_box);
  }

  if (inline_boxes.size() == 1) {
    child_boxes.push_back(inline_boxes.front());
  } else if (!inline_boxes.empty()) {
    child_boxes.push_back(CreateAnonymousBox(inline_boxes));
    inline_boxes.clear();
  }

  const auto container_box = BoxFor(element)->as<ContainerBox>();
  DCHECK(container_box) << "No container box for " << element << ". It is "
                        << BoxFor(element);
  for (auto& child_box : child_boxes)
    BoxEditor().AppendChild(container_box, child_box);
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

void BoxTree::Impl::RegisterBoxFor(Context* context,
                                   const Node& node,
                                   std::unique_ptr<Box> box) {
  IncrementVersionIfNeeded(context);
  box_map_.emplace(&node, std::move(box));
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
  for (const auto& child : container.child_nodes())
    UpdateNode(context, *child);
  AssignBoxToNode(context, container);
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
  for (const auto& child : container.child_nodes())
    UpdateNodeIfNeeded(context, *child);
  AssignBoxToNode(context, container);
}

void BoxTree::Impl::UpdateIfNeeded() {
  DCHECK_NE(BoxTreeState::Updating, state_);
  if (state_ == BoxTreeState::Clean) {
#if !DCHECK_IS_ON()
    DCHECK_EQ(style_tree_.media().viewport_size(), root_box_->viewport_size());
    for (const auto& node : Node::DescendantsOrSelf(document_)) {
      const auto box = BoxFor(node);
      DCHECK(!box->is_changed());
      const auto container_box = box->as<ContainerBox>();
      if (!container)
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
