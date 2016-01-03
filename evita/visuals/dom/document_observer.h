// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_
#define EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_

#include "base/macros.h"

namespace visuals {

namespace css {
class Style;
}

class ContainerNode;
class Element;
class Node;

//////////////////////////////////////////////////////////////////////
//
// DocumentObserver
//
class DocumentObserver {
 public:
  virtual ~DocumentObserver();

  virtual void DidAppendChild(const ContainerNode& parent,
                              const Node& child) = 0;
  virtual void DidChangeInlineStyle(const Element& element,
                                    const css::Style* old_style) = 0;
  virtual void DidInsertBefore(const ContainerNode& parent,
                               const Node& child,
                               const Node& ref_child) = 0;
  virtual void DidRemoveChild(const ContainerNode& parent,
                              const Node& child) = 0;
  virtual void WillRemoveChild(const ContainerNode& parent,
                               const Node& child) = 0;

 protected:
  DocumentObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(DocumentObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_
