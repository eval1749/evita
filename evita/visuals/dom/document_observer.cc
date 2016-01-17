// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document_observer.h"

#include "evita/visuals/dom/node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DocumentObserver
//
DocumentObserver::DocumentObserver() {}
DocumentObserver::~DocumentObserver() {}

void DocumentObserver::DidAddClass(const ElementNode& element,
                                   const base::string16& new_name) {}
void DocumentObserver::DidAppendChild(const ContainerNode& parent,
                                      const Node& child) {}
void DocumentObserver::DidChangeInlineStyle(const ElementNode& element,
                                            const css::Style* old_style) {}
void DocumentObserver::DidInsertBefore(const ContainerNode& parent,
                                       const Node& child,
                                       const Node& ref_child) {}
void DocumentObserver::DidRemoveChild(const ContainerNode& parent,
                                      const Node& child) {}
void DocumentObserver::DidRemoveClass(const ElementNode& element,
                                      const base::string16& old_name) {}
void DocumentObserver::DidReplaceChild(const ContainerNode& parent,
                                       const Node& new_child,
                                       const Node& old_child) {
  DidRemoveChild(parent, old_child);
  if (auto const next_sibling = new_child.next_sibling())
    return DidInsertBefore(parent, new_child, *next_sibling);
  DidAppendChild(parent, new_child);
}
void DocumentObserver::DidSetTextData(const Text& text,
                                      const base::string16& new_data,
                                      const base::string16& old_data) {}
void DocumentObserver::WillRemoveChild(const ContainerNode& parent,
                                       const Node& child) {}

}  // namespace visuals
