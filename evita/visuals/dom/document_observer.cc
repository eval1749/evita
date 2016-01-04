// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document_observer.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DocumentObserver
//
DocumentObserver::DocumentObserver() {}
DocumentObserver::~DocumentObserver() {}

void DocumentObserver::DidAddClass(const Element& element,
                                   const base::string16& new_name) {}
void DocumentObserver::DidAppendChild(const ContainerNode& parent,
                                      const Node& child) {}
void DocumentObserver::DidChangeInlineStyle(const Element& element,
                                            const css::Style* old_style) {}
void DocumentObserver::DidInsertBefore(const ContainerNode& parent,
                                       const Node& child,
                                       const Node& ref_child) {}
void DocumentObserver::DidRemoveChild(const ContainerNode& parent,
                                      const Node& child) {}
void DocumentObserver::DidRemoveClass(const Element& element,
                                      const base::string16& old_name) {}
void DocumentObserver::WillRemoveChild(const ContainerNode& parent,
                                       const Node& child) {}

}  // namespace visuals
