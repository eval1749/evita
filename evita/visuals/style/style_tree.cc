// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <iterator>
#include <ostream>

#include "evita/visuals/style/style_tree.h"

#include "base/observer_list.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/css/style_sheet_observer.h"
#include "evita/visuals/dom/ancestors.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/style/compiled_style_sheet.h"
#include "evita/visuals/style/style_tree_observer.h"

namespace visuals {

namespace {
struct Item {
  bool is_child_dirty = true;
  bool is_dirty = true;
  std::unique_ptr<css::Style> style;
  int version;
};

void InheritStyle(css::Style* style, const css::Style& parent_style) {
  if (!style->has_color() && parent_style.has_color())
    css::StyleEditor().SetColor(style, parent_style.color());
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// StyleTreeState
//
#define FOR_EACH_STYLE_TREE_STATE(V) \
  V(Clean)                           \
  V(Dirty)                           \
  V(Updating)

enum class StyleTreeState {
#define V(name) name,
  FOR_EACH_STYLE_TREE_STATE(V)
#undef V
};

std::ostream& operator<<(std::ostream& ostream, StyleTreeState state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_STYLE_TREE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

//////////////////////////////////////////////////////////////////////
//
// StyleTree::Impl
//
class StyleTree::Impl final {
 public:
  Impl(const Document& document,
       const css::Media& media,
       const std::vector<css::StyleSheet*>& style_sheets);
  ~Impl() = default;

  const Document& document() const { return document_; }
  const css::Style& initial_style() const { return *initial_style_; }
  bool is_dirty() const;
  const css::Media& media() const { return media_; }
  int version() const { return version_; }

  void AddObserver(StyleTreeObserver* observer);
  void Clear();
  const css::Style& ComputedStyleOf(const Node& node) const;
  void MarkDirty(const Node& node);
  void RemoveObserver(StyleTreeObserver* observer);
  void UpdateIfNeeded();

 private:
  struct Context {
    bool is_updated = false;
  };

  std::unique_ptr<css::Style> ComputeStyleForDocument() const;
  std::unique_ptr<css::Style> ComputeStyleForElement(
      const Element& element) const;
  Item* GetOrNewItem(const Node& element);
  void IncrementVersionIfNeeded(Context* context);
  void UpdateChildren(Context* context, const ContainerNode& element);
  void UpdateDocumentStyleIfNeeded(Context* context);
  void UpdateElement(Context* context, const Element& element);
  void UpdateElementIfNeeded(Context* context, const Element& element);
  void UpdateNodeIfNeeded(Context* context, const Node& node);
  void UpdateText(Context* context, const Text& text);

  std::vector<std::unique_ptr<CompiledStyleSheet>> compiled_style_sheets_;
  const Document& document_;
  // |initial_style_| is computed from media provided values.
  css::Style* initial_style_ = nullptr;
  // TODO(eval1749): We should share |css::Style| objects for elements which
  // have same style, e.g. siblings.
  std::unordered_map<const Node*, std::unique_ptr<Item>> item_map_;
  const css::Media& media_;
  base::ObserverList<StyleTreeObserver> observers_;
  StyleTreeState state_ = StyleTreeState::Dirty;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

StyleTree::Impl::Impl(const Document& document,
                      const css::Media& media,
                      const std::vector<css::StyleSheet*>& style_sheets)
    : document_(document), media_(media) {
  for (const auto& style_sheet : style_sheets)
    compiled_style_sheets_.emplace_back(new CompiledStyleSheet(*style_sheet));
}

bool StyleTree::Impl::is_dirty() const {
  DCHECK_NE(StyleTreeState::Updating, state_);
  return state_ == StyleTreeState::Dirty;
}

void StyleTree::Impl::AddObserver(StyleTreeObserver* observer) {
  observers_.AddObserver(observer);
}

void StyleTree::Impl::Clear() {
  state_ = StyleTreeState::Dirty;
}

const css::Style& StyleTree::Impl::ComputedStyleOf(const Node& node) const {
  DCHECK_NE(StyleTreeState::Dirty, state_);
  const auto& it = item_map_.find(&node);
  if (it == item_map_.end()) {
    NOTREACHED() << "No computed style for " << node;
    return *initial_style_;
  }
  return *it->second->style;
}

std::unique_ptr<css::Style> StyleTree::Impl::ComputeStyleForDocument() const {
  DCHECK_NE(StyleTreeState::Dirty, state_);
  auto style = std::make_unique<css::Style>();
  // TODO(eval1749): We should get default color and background color from
  // system metrics.
  css::StyleEditor().SetBackground(style.get(), css::Background(css::Color()));
  css::StyleEditor().SetColor(style.get(), css::Color(0, 0, 0));
  css::StyleEditor().SetDisplay(style.get(), css::Display());
  DCHECK(style.get()->has_display())
      << "initial style must have display property. " << initial_style();
  return std::move(style);
}

std::unique_ptr<css::Style> StyleTree::Impl::ComputeStyleForElement(
    const Element& element) const {
  DCHECK_NE(StyleTreeState::Dirty, state_);
  const auto inline_style = element.inline_style();
  auto style = inline_style ? std::make_unique<css::Style>(*inline_style)
                            : std::make_unique<css::Style>();
  for (const auto& style_sheet : compiled_style_sheets_) {
    auto matched = style_sheet->Match(element);
    if (!matched)
      continue;
    css::StyleEditor().Merge(style.get(), *matched);
  }
  InheritStyle(style.get(), ComputedStyleOf(*element.parent()));
  css::StyleEditor().Merge(style.get(), initial_style());
  DCHECK(style->has_display()) << "A style must have display property. "
                               << initial_style();
  return std::move(style);
}

Item* StyleTree::Impl::GetOrNewItem(const Node& node) {
  const auto& it = item_map_.find(&node);
  if (it != item_map_.end())
    return it->second.get();
  // TODO(eval1749): We should notify style change to observers.
  auto new_item = std::make_unique<Item>();
  new_item->version = version_;
  const auto& result = item_map_.emplace(&node, std::move(new_item));
  return result.first->second.get();
}

void StyleTree::Impl::IncrementVersionIfNeeded(Context* context) {
  if (context->is_updated)
    return;
  context->is_updated = true;
  ++version_;
}

void StyleTree::Impl::MarkDirty(const Node& node) {
  state_ = StyleTreeState::Dirty;
  GetOrNewItem(node)->is_dirty = true;
  for (const auto& ancestor : Node::Ancestors(node)) {
    const auto item = GetOrNewItem(*ancestor);
    if (item->is_child_dirty)
      return;
    item->is_child_dirty = true;
  }
}

void StyleTree::Impl::RemoveObserver(StyleTreeObserver* observer) {
  observers_.RemoveObserver(observer);
}

void StyleTree::Impl::UpdateChildren(Context* context,
                                     const ContainerNode& container) {
  for (const auto& child : container.child_nodes()) {
    if (const auto element = child->as<Element>()) {
      UpdateElement(context, *element);
      continue;
    }
    if (const auto text = child->as<Text>()) {
      UpdateText(context, *text);
      continue;
    }
    NOTREACHED() << "Unsupported node type " << child;
  }
}

void StyleTree::Impl::UpdateDocumentStyleIfNeeded(Context* context) {
  const auto item = GetOrNewItem(document_);
  if (!item->is_dirty) {
    if (!item->is_child_dirty)
      return;
    for (const auto& child : document_.child_nodes())
      UpdateNodeIfNeeded(context, *child);
    return;
  }
  IncrementVersionIfNeeded(context);
  item->is_child_dirty = false;
  item->is_dirty = false;
  const auto old_style = std::move(item->style);
  item->style = std::move(ComputeStyleForDocument());
  item->version = version_;
  initial_style_ = item->style.get();
  UpdateChildren(context, document_);
}

void StyleTree::Impl::UpdateElement(Context* context, const Element& element) {
  IncrementVersionIfNeeded(context);
  const auto item = GetOrNewItem(element);
  item->is_child_dirty = false;
  item->is_dirty = false;
  const auto old_style = std::move(item->style);
  item->style = std::move(ComputeStyleForElement(element));
  item->version = version_;
  // Simple style merge check.
  DCHECK(item->style->has_display())
      << "Style merge failed. We should merge initial style. " << *item->style;
  if (old_style) {
    FOR_EACH_OBSERVER(StyleTreeObserver, observers_,
                      DidChangeComputedStyle(element, *old_style));
  }
  UpdateChildren(context, element);
}

void StyleTree::Impl::UpdateElementIfNeeded(Context* context,
                                            const Element& element) {
  const auto item = GetOrNewItem(element);
  if (item->is_dirty)
    return UpdateElement(context, element);
  if (!item->is_child_dirty)
    return;
  item->is_child_dirty = false;
  for (const auto& child : element.child_nodes())
    UpdateNodeIfNeeded(context, *child);
}

// The entry point
void StyleTree::Impl::UpdateIfNeeded() {
  if (state_ == StyleTreeState::Clean) {
#if !DCHECK_IS_ON()
    // All style tree node should be clean.
    for (const auto& node : Node::DescendantsOrSelf(document_)) {
      const auto element = node->as<Element>();
      if (!element)
        continue;
      const auto& it = item_map_.find(element);
      DCHECK(it != item_map_.end());
      const auto& item = it->second;
      DCHECK(!item->is_dirty);
      DCHECK(!item->is_child_dirty);
    }
#endif
    return;
  }
  state_ = StyleTreeState::Updating;
  Context context;
  UpdateDocumentStyleIfNeeded(&context);
  DCHECK(context.is_updated);
  state_ = StyleTreeState::Clean;
}

void StyleTree::Impl::UpdateNodeIfNeeded(Context* context, const Node& node) {
  if (const auto element = node.as<Element>())
    return UpdateElementIfNeeded(context, *element);
  DCHECK(!node.is<ContainerNode>()) << "Unsupported node type " << node;
}

// |Text| node is treated as anonymous inline box which inherits style from
// its parent.
void StyleTree::Impl::UpdateText(Context* context, const Text& text) {
  const auto item = GetOrNewItem(text);
  item->is_dirty = false;
  item->is_child_dirty = false;
  auto style = std::make_unique<css::Style>();
  InheritStyle(style.get(), ComputedStyleOf(*text.parent()));
  css::StyleEditor().Merge(style.get(), initial_style());
  item->style = std::move(style);
}

//////////////////////////////////////////////////////////////////////
//
// StyleTree
//
StyleTree::StyleTree(const Document& document,
                     const css::Media& media,
                     const std::vector<css::StyleSheet*>& style_sheets)
    : impl_(new Impl(document, media, style_sheets)),
      style_sheets_(style_sheets) {
  document.AddObserver(this);
  media.AddObserver(this);
  for (const auto& style_sheet : style_sheets_)
    style_sheet->AddObserver(this);
}

StyleTree::~StyleTree() {
  for (const auto& style_sheet : style_sheets_)
    style_sheet->RemoveObserver(this);
  impl_->document().RemoveObserver(this);
  impl_->media().RemoveObserver(this);
}

const Document& StyleTree::document() const {
  return impl_->document();
}

const css::Style& StyleTree::initial_style() const {
  return impl_->initial_style();
}

const css::Media& StyleTree::media() const {
  return impl_->media();
}

int StyleTree::version() const {
  return impl_->version();
}

void StyleTree::AddObserver(StyleTreeObserver* observer) const {
  impl_->AddObserver(observer);
}

void StyleTree::Clear() {
  impl_->Clear();
}

const css::Style& StyleTree::ComputedStyleOf(const Node& node) const {
  DCHECK(!impl_->is_dirty()) << "You should call "
                                "StyleTree::UpdateIfNeeded(), before "
                                "calling ComputedStyleOf().";
  return impl_->ComputedStyleOf(node);
}

void StyleTree::RemoveObserver(StyleTreeObserver* observer) const {
  impl_->RemoveObserver(observer);
}

void StyleTree::UpdateIfNeeded() {
  impl_->UpdateIfNeeded();
}

// css::MediaObserver
void StyleTree::DidChangeViewportSize() {
  // Note: viewport size change doesn't affect compute style since we don't
  // have media query.
}

void StyleTree::DidChangeSystemMetrics() {
  // TODO(eval1749): Invalidate styles using system colors.
  Clear();
}

// css::StyleSheetObserver
void StyleTree::DidAddRule(const css::Rule& rule) {
  Clear();
}

void StyleTree::DidRemoveRule(const css::Rule& rule) {
  Clear();
}

// DocumentObserver
void StyleTree::DidAddClass(const Element& element,
                            const base::string16& name) {
  // TODO(eval1749): Implement shortcut for
  //  - position:absolute + left/top
  //  - background color change
  //  - border color change
  // Since above changes don't affect layout.
  impl_->MarkDirty(element);
}

void StyleTree::DidAppendChild(const ContainerNode& parent, const Node& child) {
  impl_->MarkDirty(parent);
}

void StyleTree::DidChangeInlineStyle(const Element& element,
                                     const css::Style* old_style) {
  // TODO(eval1749): Implement shortcut for
  //  - position:absolute + left/top
  //  - background color change
  //  - border color change
  // Since above changes don't affect layout.
  impl_->MarkDirty(element);
}

void StyleTree::DidInsertBefore(const ContainerNode& parent,
                                const Node& child,
                                const Node& ref_child) {
  impl_->MarkDirty(parent);
}

void StyleTree::DidRemoveChild(const ContainerNode& parent, const Node& child) {
  impl_->MarkDirty(parent);
}

}  // namespace visuals
