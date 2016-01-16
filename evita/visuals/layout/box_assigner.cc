// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_assigner.h"

#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_map.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

namespace {
// TODO(eval1749) We should move |IsDisplayOutsideInline()| to |css::Display|.
bool IsDisplayInsideFlow(const css::Display& display) {
  return display.is_block() || display.is_inline() || display.is_inline_block();
}
}

//////////////////////////////////////////////////////////////////////
//
// BoxAssigner
//
BoxAssigner::BoxAssigner(BoxMap* box_map) : box_map_(box_map) {}
BoxAssigner::~BoxAssigner() {}

RootBox* BoxAssigner::root_box() const {
  return box_map_->root_box_;
}

void BoxAssigner::Assign(const Node& node, const css::Style& style) {
  DCHECK(!style_);
  style_ = &style;
  Visit(node);
  style_ = nullptr;
}

Box* BoxAssigner::BoxFor(const Node& node) const {
  return box_map_->BoxFor(node);
}

void BoxAssigner::DetachChildren(ContainerBox* container) {
  while (const auto child = container->first_child()) {
    BoxEditor().RemoveChild(container, child);
    if (child->node())
      continue;
    // Delete anonymous box.
    if (const auto anonymous_container = child->as<ContainerBox>())
      DetachChildren(anonymous_container);
    delete child;
  }
}

void BoxAssigner::RegisterBoxFor(const Node& node, std::unique_ptr<Box> box) {
  const auto& result = box_map_->map_.emplace(&node, std::move(box));
  DCHECK(result.second) << node << " is already in box_map_->map_";
}

// NodeVisitor
void BoxAssigner::VisitDocument(Document* document) {
  DCHECK_EQ(root_box()->node(), document);
}

void BoxAssigner::VisitElement(Element* element) {
  const auto& it = box_map_->map_.find(element);
  if (style_->display().is_none()) {
    if (it == box_map_->map_.end())
      return;
    DetachChildren(it->second->as<ContainerBox>());
    box_map_->map_.erase(it);
    return;
  }
  if (it != box_map_->map_.end()) {
    const auto container_box = it->second->as<ContainerBox>();
    DetachChildren(container_box);
    if (container_box->display() == style_->display()) {
      BoxEditor().SetStyle(container_box, *style_);
      return;
    }
    box_map_->map_.erase(it);
  }

  if (!IsDisplayInsideFlow(style_->display())) {
    NOTREACHED() << "Unsupported display:" << style_->display() << " of "
                 << *element;
    return;
  }

  // TODO(eval1749): |Box| constructor should take |css::Style|.
  auto new_flow_box = std::make_unique<FlowBox>(root_box(), element);
  BoxEditor().SetStyle(new_flow_box.get(), *style_);
  RegisterBoxFor(*element, std::move(new_flow_box));
}

void BoxAssigner::VisitText(Text* text) {
  if (const auto text_box = BoxFor(*text)) {
    BoxEditor().SetTextData(text_box->as<TextBox>(), text->data());
    BoxEditor().SetStyle(text_box, *style_);
    return;
  }
  // TODO(eval1749): We should make |TextBox| constructor to take color.
  // TODO(eval1749): |TextBox| constructor should take |const Text&|.
  auto new_text_box = std::make_unique<TextBox>(root_box(), text->data(), text);
  BoxEditor().SetStyle(new_text_box.get(), *style_);
  RegisterBoxFor(*text, std::move(new_text_box));
}

}  // namespace visuals
