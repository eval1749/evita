// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_tree.h"

#include "base/observer_list.h"
#include "evita/visuals/css/media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/css/style_sheet_observer.h"
#include "evita/visuals/dom/ancestors.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
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
// StyleTree::Impl
//
class StyleTree::Impl final {
 public:
  explicit Impl(const std::vector<css::StyleSheet*>& style_sheets);
  ~Impl() = default;

  const css::Style& initial_style() const { return *initial_style_; }
  bool is_dirty() const { return is_dirty_; }
  int version() const { return version_; }

  void AddObserver(StyleTreeObserver* observer);
  void Clear();
  const css::Style& ComputedStyleOf(const Node& node) const;
  void MarkDirty(const Element& element);
  void RemoveObserver(StyleTreeObserver* observer);
  void UpdateIfNeeded(const Document& document);

 private:
  struct Context {
    bool is_updated = false;
  };

  std::unique_ptr<css::Style> ComputeStyleForElement(
      const Element& element) const;
  Item* GetOrNewItem(const Element& element);
  const css::Style& InlineStyleOf(const Element& element) const;
  void UpdateChildren(Context* context, const Element& element);
  void UpdateElement(Context* context, const Element& element);
  void UpdateElementIfNeeded(Context* context, const Element& element);
  void UpdateNodeIfNeeded(Context* context, const Node& node);

  std::vector<std::unique_ptr<CompiledStyleSheet>> compiled_style_sheets_;
  // |initial_style_| is computed from media provided values.
  std::unique_ptr<css::Style> initial_style_;
  // TODO(eval1749): We should share |css::Style| objects for elements which
  // have same style, e.g. siblings.
  std::unordered_map<const Element*, std::unique_ptr<Item>> item_map_;
  bool is_dirty_ = true;
  base::ObserverList<StyleTreeObserver> observers_;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

StyleTree::Impl::Impl(const std::vector<css::StyleSheet*>& style_sheets)
    : initial_style_(new css::Style()) {
  // TODO(eval1749): We should get default color and background color from
  // system metrics.
  css::StyleEditor().SetBackground(initial_style_.get(),
                                   css::Background(css::Color()));
  css::StyleEditor().SetColor(initial_style_.get(), css::Color(0, 0, 0));
  css::StyleEditor().SetDisplay(initial_style_.get(), css::Display());
  for (const auto& style_sheet : style_sheets)
    compiled_style_sheets_.emplace_back(new CompiledStyleSheet(*style_sheet));
  DCHECK(initial_style().has_display())
      << "initial style must have display property. " << initial_style();
}

void StyleTree::Impl::AddObserver(StyleTreeObserver* observer) {
  observers_.AddObserver(observer);
}

void StyleTree::Impl::Clear() {
  item_map_.clear();
  is_dirty_ = true;
  FOR_EACH_OBSERVER(StyleTreeObserver, observers_, DidClearStyleCache());
}

const css::Style& StyleTree::Impl::ComputedStyleOf(const Node& node) const {
  if (const auto element = node.as<Element>()) {
    const auto& it = item_map_.find(element);
    DCHECK(it != item_map_.end());
    return *it->second->style;
  }
  if (node.is<Document>())
    return *initial_style_;
  return ComputedStyleOf(*node.parent());
}

std::unique_ptr<css::Style> StyleTree::Impl::ComputeStyleForElement(
    const Element& element) const {
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

Item* StyleTree::Impl::GetOrNewItem(const Element& element) {
  const auto& it = item_map_.find(&element);
  if (it != item_map_.end())
    return it->second.get();
  // TODO(eval1749): We should notify style change to observers.
  auto new_item = std::make_unique<Item>();
  const auto& result = item_map_.emplace(&element, std::move(new_item));
  return result.first->second.get();
}

void StyleTree::Impl::MarkDirty(const Element& element) {
  is_dirty_ = true;
  GetOrNewItem(element)->is_dirty = true;
  for (const auto& ancestor : Node::Ancestors(element)) {
    const auto ancestor_element = ancestor->as<Element>();
    if (!ancestor_element)
      return;
    const auto item = GetOrNewItem(*ancestor_element);
    if (item->is_child_dirty)
      return;
    item->is_child_dirty = true;
  }
}

void StyleTree::Impl::RemoveObserver(StyleTreeObserver* observer) {
  observers_.RemoveObserver(observer);
}

void StyleTree::Impl::UpdateChildren(Context* context, const Element& element) {
  for (const auto& child : element.child_nodes()) {
    const auto child_element = child->as<Element>();
    if (!child_element)
      continue;
    UpdateElement(context, *child_element);
  }
}

void StyleTree::Impl::UpdateElement(Context* context, const Element& element) {
  if (!context->is_updated) {
    context->is_updated = true;
    ++version_;
  }
  const auto item = GetOrNewItem(element);
  item->is_child_dirty = false;
  item->is_dirty = false;
  item->style = std::move(ComputeStyleForElement(element));
  item->version = version_;
  // Simple style merge check.
  DCHECK(item->style->has_display())
      << "Style merge failed. We should merge initial style. " << *item->style;
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
void StyleTree::Impl::UpdateIfNeeded(const Document& document) {
#if !DCHECK_IS_ON()
  if (!is_dirty_)
    return;
#endif
  Context context;
  for (const auto& child : document.child_nodes())
    UpdateNodeIfNeeded(&context, *child);
  DCHECK_EQ(is_dirty_, context.is_updated);
  is_dirty_ = false;
}

void StyleTree::Impl::UpdateNodeIfNeeded(Context* context, const Node& node) {
  const auto element = node.as<Element>();
  if (!element)
    return;
  UpdateElementIfNeeded(context, *element);
}

//////////////////////////////////////////////////////////////////////
//
// StyleTree
//
StyleTree::StyleTree(const Document& document,
                     const css::Media& media,
                     const std::vector<css::StyleSheet*>& style_sheets)
    : document_(document),
      impl_(new Impl(style_sheets)),
      media_(media),
      style_sheets_(style_sheets) {
  document_.AddObserver(this);
  media_.AddObserver(this);
  for (const auto& style_sheet : style_sheets_)
    style_sheet->AddObserver(this);
}

StyleTree::~StyleTree() {
  for (const auto& style_sheet : style_sheets_)
    style_sheet->RemoveObserver(this);
  document_.RemoveObserver(this);
}

const css::Style& StyleTree::initial_style() const {
  return impl_->initial_style();
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
  impl_->UpdateIfNeeded(document_);
}

// css::MediaObserver
void StyleTree::DidChangeViewportSize() {
  // TODO(eval1749): Invalidate styles depends on viewport size
  Clear();
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

void StyleTree::DidChangeInlineStyle(const Element& element,
                                     const css::Style* old_style) {
  // TODO(eval1749): Implement shortcut for
  //  - position:absolute + left/top
  //  - background color change
  //  - border color change
  // Since above changes don't affect layout.
  impl_->MarkDirty(element);
}

}  // namespace visuals
