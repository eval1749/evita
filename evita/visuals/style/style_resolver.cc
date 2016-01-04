// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_resolver.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/style/compiled_style_sheet.h"
#include "evita/visuals/style/style_change_observer.h"

namespace visuals {

namespace {
void InheritStyle(css::Style* style, const css::Style& parent_style) {
  if (!style->has_color() && parent_style.has_color())
    css::StyleEditor().SetColor(style, parent_style.color());
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// StyleResolver
//
StyleResolver::StyleResolver(const Document& document,
                             const css::Media& media,
                             const std::vector<css::StyleSheet*>& style_sheets)
    : default_style_(new css::Style()),
      document_(document),
      media_(media),
      style_sheets_(style_sheets) {
  // TODO(eval1749): We should get default color and background color from
  // system metrics.
  css::StyleEditor().SetBackground(default_style_.get(),
                                   css::Background(css::Color()));
  css::StyleEditor().SetColor(default_style_.get(), css::Color(0, 0, 0));
  css::StyleEditor().SetDisplay(default_style_.get(), css::Display());
  document_.AddObserver(this);
  for (const auto& style_sheet : style_sheets_) {
    style_sheet->AddObserver(this);
    compiled_style_sheets_.emplace_back(new CompiledStyleSheet(*style_sheet));
  }
}

StyleResolver::~StyleResolver() {
  for (const auto& style_sheet : style_sheets_)
    style_sheet->RemoveObserver(this);
  document_.RemoveObserver(this);
}

void StyleResolver::AddObserver(StyleChangeObserver* observer) const {
  observers_.AddObserver(observer);
}

void StyleResolver::Clear() {
  style_map_.clear();
  FOR_EACH_OBSERVER(StyleChangeObserver, observers_, DidClearStyleCache());
}

std::unique_ptr<css::Style> StyleResolver::ComputeStyleFor(
    const Element& element) {
  const auto inline_style = element.inline_style();
  auto style = inline_style ? std::make_unique<css::Style>(*inline_style)
                            : std::make_unique<css::Style>();
  for (const auto& style_sheet : compiled_style_sheets_) {
    auto matched = style_sheet->Match(element);
    if (!matched)
      continue;
    css::StyleEditor().Merge(style.get(), *matched);
  }
  InheritStyle(style.get(), ResolveFor(*element.parent()));
  return std::move(style);
}

const css::Style& StyleResolver::InlineStyleOf(const Element& element) const {
  if (const auto inline_style = element.inline_style())
    return *inline_style;
  return *default_style_;
}

void StyleResolver::RemoveObserver(StyleChangeObserver* observer) const {
  observers_.RemoveObserver(observer);
}

const css::Style& StyleResolver::ResolveFor(const Node& node) {
  Document::LockScope lock_scope(document_);
  if (node.is<Document>())
    return *default_style_;
  const auto element = node.as<Element>();
  if (!element)
    return ResolveFor(*node.parent());
  const auto& it = style_map_.find(element);
  if (it != style_map_.end())
    return *it->second;
  auto style = ComputeStyleFor(*element);
  css::StyleEditor().Merge(style.get(), *default_style_);
  const auto& result = style_map_.emplace(element, std::move(style));
  return *result.first->second;
}

// css::MediaObserver
void StyleResolver::DidChangeViewportSize() {
  // TODO(eval1749): Invalidate styles depends on viewport size
  Clear();
}

void StyleResolver::DidChangeSystemMetrics() {
  // TODO(eval1749): Invalidate styles using system colors.
  Clear();
}

// css::StyleSheetObserver
void StyleResolver::DidAddRule(const css::Rule& rule) {
  Clear();
}

void StyleResolver::DidRemoveRule(const css::Rule& rule) {
  Clear();
}

// DocumentObserver
void StyleResolver::DidChangeInlineStyle(const Element& element,
                                         const css::Style* old_style) {
  const auto& it = style_map_.find(&element);
  if (it == style_map_.end())
    return;
  FOR_EACH_OBSERVER(StyleChangeObserver, observers_,
                    DidRemoveStyleCache(element, *it->second));
  style_map_.erase(it);
}

void StyleResolver::WillRemoveChild(const ContainerNode& parent,
                                    const Node& child) {
  const auto element = child.as<Element>();
  if (!element)
    return;
  const auto& it = style_map_.find(element);
  if (it == style_map_.end())
    return;
  style_map_.erase(it);
}

}  // namespace visuals
