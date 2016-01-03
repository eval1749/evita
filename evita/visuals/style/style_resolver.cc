// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_resolver.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_editor.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"

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
StyleResolver::StyleResolver(const Document& document)
    : default_style_(new css::Style()), document_(document) {
  css::StyleEditor().SetColor(default_style_.get(), css::Color(0, 0, 0));
  document_.AddObserver(this);
}

StyleResolver::~StyleResolver() {
  document_.RemoveObserver(this);
}

std::unique_ptr<css::Style> StyleResolver::ComputeStyleFor(
    const Element& element) {
  const auto& parent_style = ResolveFor(*element.parent());
  if (const auto inline_style = element.inline_style()) {
    auto style = std::make_unique<css::Style>(*inline_style);
    InheritStyle(style.get(), parent_style);
    return std::move(style);
  }
  return std::make_unique<css::Style>(parent_style);
}

const css::Style& StyleResolver::InlineStyleOf(const Element& element) const {
  if (const auto inline_style = element.inline_style())
    return *inline_style;
  return *default_style_;
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
  const auto& result = style_map_.emplace(element, std::move(style));
  return *result.first->second;
}

// DocumentObserver
void StyleResolver::DidChangeInlineStyle(const Element& element,
                                         const css::Style* old_style) {
  const auto& it = style_map_.find(&element);
  if (it == style_map_.end())
    return;
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
