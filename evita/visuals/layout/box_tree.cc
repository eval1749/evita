// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <ostream>
#include <vector>

#include "evita/visuals/layout/box_tree.h"

#include "base/logging.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/selection.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/box_assigner.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_map.h"
#include "evita/visuals/layout/box_selection.h"
#include "evita/visuals/layout/box_selection_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/view/public/view_lifecycle.h"

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
class BoxTree::Impl final {
 public:
  Impl(ViewLifecycle* lifecycle, const StyleTree& style_tree);
  ~Impl() = default;

  const Document& document() const { return style_tree_.document(); }
  const css::Media& media() const { return style_tree_.media(); }
  RootBox* root_box() const;
  const StyleTree& style_tree() const { return style_tree_; }
  int version() const { return version_; }

  Box* BoxFor(const Node& node) const;
  bool IsClean() const { return state_ == BoxTreeState::Clean; }
  void MarkDirty(const Node& node);
  void UpdateIfNeeded();

 private:
  void AssignBoxToNode(Context* context, const Node& node);
  const css::Style& ComputedStyleOf(const Node& node) const;
  std::unique_ptr<FlowBox> CreateAnonymousBlockBox(
      const std::vector<Box*>& inline_boxes);
  void FormatContainerBox(Context* context, ContainerBox* container_box);
  void FormatFlowBox(Context* context, FlowBox* flow_box);
  void FormatRootBox(Context* context, RootBox* root_box);
  void IncrementVersionIfNeeded(Context* context);
  void UpdateContainerNode(Context* context, const ContainerNode& container);
  void UpdateContainerNodeIfNeeded(Context* context,
                                   const ContainerNode& container);
  void UpdateNode(Context* context, const Node& node);
  void UpdateNodeIfNeeded(Context* context, const Node& node);

  BoxMap box_map_;
  const Document& document_;
  RootBox* const root_box_;
  BoxTreeState state_;
  const StyleTree& style_tree_;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

BoxTree::Impl::Impl(ViewLifecycle* lifecycle, const StyleTree& style_tree)
    : box_map_(lifecycle),
      document_(lifecycle->document()),
      root_box_(box_map_.root_box()),
      state_(BoxTreeState::Dirty),
      style_tree_(style_tree) {}

RootBox* BoxTree::Impl::root_box() const {
  DCHECK_NE(BoxTreeState::Updating, state_);
  DCHECK_EQ(BoxTreeState::Clean, state_);
  return root_box_;
}

void BoxTree::Impl::AssignBoxToNode(Context* context, const Node& node) {
  IncrementVersionIfNeeded(context);
  BoxAssigner(&box_map_).Assign(node, ComputedStyleOf(node));
}

Box* BoxTree::Impl::BoxFor(const Node& node) const {
  DCHECK_NE(BoxTreeState::Dirty, state_);
  return box_map_.BoxFor(node);
}

const css::Style& BoxTree::Impl::ComputedStyleOf(const Node& node) const {
  return style_tree_.ComputedStyleOf(node);
}

std::unique_ptr<FlowBox> BoxTree::Impl::CreateAnonymousBlockBox(
    const std::vector<Box*>& inline_boxes) {
  auto block_box = std::make_unique<FlowBox>(root_box_);
  BoxEditor().SetDisplay(block_box.get(), css::Display::Block());
  for (const auto& inline_box : inline_boxes)
    BoxEditor().AppendChild(block_box.get(), inline_box);
  return std::move(block_box);
}

// Full form of display-legacy
//  block           block flow
//  inline          inline flow
//  inline-block    inline flow-root
void BoxTree::Impl::FormatContainerBox(Context* context,
                                       ContainerBox* container_box) {
  if (auto const root_box = container_box->as<RootBox>())
    return FormatRootBox(context, root_box);
  if (auto const flow_box = container_box->as<FlowBox>())
    return FormatFlowBox(context, flow_box);
  NOTREACHED() << "Unsupported layout type of box " << *container_box;
}

void BoxTree::Impl::FormatFlowBox(Context* context, FlowBox* flow_box) {
  DCHECK(flow_box->node()->is<Element>());
  std::vector<Box*> block_boxes;
  std::vector<Box*> inline_boxes;
  for (const auto& child :
       flow_box->node()->as<ContainerNode>()->child_nodes()) {
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
      block_boxes.push_back(CreateAnonymousBlockBox(inline_boxes).release());
      inline_boxes.clear();
    }
    block_boxes.push_back(child_box);
  }

  if (block_boxes.empty() && inline_boxes.empty())
    return;

  if (block_boxes.empty()) {
    for (const auto& inline_box : inline_boxes)
      BoxEditor().AppendChild(flow_box, inline_box);
    return;
  }

  if (!inline_boxes.empty()) {
    block_boxes.push_back(CreateAnonymousBlockBox(inline_boxes).release());
    inline_boxes.clear();
  }

  for (const auto& block_box : block_boxes)
    BoxEditor().AppendChild(flow_box, block_box);
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
  // Note: When |BoxTree| is dirty, we can't use |BoxTree::Impl::BoxFor()|.
  const auto box = box_map_.BoxFor(node);
  if (!box)
    return;
  BoxEditor().MarkDirty(box);
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
  BoxEditor().SetStyle(root_box_, ComputedStyleOf(document_));
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

//////////////////////////////////////////////////////////////////////
//
// BoxTree
//
BoxTree::BoxTree(ViewLifecycle* lifecycle,
                 const Selection& selection,
                 const StyleTree& style_tree)
    : impl_(new Impl(lifecycle, style_tree)), selection_(selection) {
  DCHECK_EQ(lifecycle->document(), selection_.document());
  impl_->document().AddObserver(this);
  impl_->media().AddObserver(this);
  impl_->style_tree().AddObserver(this);
  selection_.AddObserver(this);
}

BoxTree::~BoxTree() {
  impl_->document().RemoveObserver(this);
  impl_->media().RemoveObserver(this);
  impl_->style_tree().RemoveObserver(this);
  selection_.RemoveObserver(this);
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

BoxSelection BoxTree::ComputeSelection() const {
  if (selection_.is_none())
    return BoxSelection();
  const auto& style = impl_->style_tree().ComputedStyleOfSelection(selection_);
  BoxSelection selection;
  if (selection_.is_caret()) {
    BoxSelectionEditor().Collapse(&selection, BoxFor(selection_.focus_node()),
                                  selection_.focus_offset());
  } else {
    BoxSelectionEditor().Collapse(&selection, BoxFor(selection_.anchor_node()),
                                  selection_.anchor_offset());
    BoxSelectionEditor().ExtendTo(&selection, BoxFor(selection_.focus_node()),
                                  selection_.focus_offset());
    BoxSelectionEditor().SetSelectionColor(&selection,
                                           style.background_color().value());
  }
  if (style.caret_shape().is_none())
    return selection;
  BoxSelectionEditor().SetCaretColor(&selection, style.caret_color().value());
  BoxSelectionEditor().SetCaretShape(&selection, style.caret_shape());
  return selection;
}

void BoxTree::ScheduleForcePaint() {
  if (!impl_->IsClean())
    return;
  BoxEditor().ScheduleForcePaint(root_box());
}

void BoxTree::UpdateIfNeeded() {
  impl_->UpdateIfNeeded();
  UpdateSelectionIfNeeded();
}

void BoxTree::UpdateSelectionIfNeeded() {
  if (!is_selection_changed_)
    return;
  BoxEditor().SetSelection(root_box(), ComputeSelection());
  is_selection_changed_ = false;
}

// css::MediaObserver
void BoxTree::DidChangeMediaState() {
  // Nothing to do
}

void BoxTree::DidChangeSystemMetrics() {
  // Note: system metrics changes affect computed style.
}

void BoxTree::DidChangeViewportSize() {
  impl_->MarkDirty(impl_->document());
}

// DocumentObserver
void BoxTree::DidAppendChild(const ContainerNode& parent, const Node& child) {
  impl_->MarkDirty(parent);
}

void BoxTree::DidChangeInlineStyle(const ElementNode& element,
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

void BoxTree::DidRemoveChild(const ContainerNode& parent, const Node& child) {
  impl_->MarkDirty(parent);
}

void BoxTree::DidReplaceChild(const ContainerNode& parent,
                              const Node& child,
                              const Node& ref_child) {
  impl_->MarkDirty(parent);
}

// SelectionObserver
void BoxTree::DidChangeCaretBlink() {
  is_selection_changed_ = true;
}

void BoxTree::DidChangeSelection(const SelectionModel& new_model,
                                 const SelectionModel& old_model) {
  is_selection_changed_ = true;
}

// StyleTreeObserver
void BoxTree::DidChangeComputedStyle(const ElementNode& element,
                                     const css::Style& old_style) {
  impl_->MarkDirty(element);
}

void BoxTree::DidSetTextData(const Text& text,
                             const base::string16& new_data,
                             const base::string16& old_data) {
  impl_->MarkDirty(text);
}

}  // namespace visuals
