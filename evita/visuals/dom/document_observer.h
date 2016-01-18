// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_
#define EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/visuals/dom/nodes_forward.h"

namespace visuals {

class ShapeData;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// DocumentObserver
//
class DocumentObserver {
 public:
  virtual ~DocumentObserver();

  virtual void DidAddClass(const ElementNode& element,
                           const base::string16& new_name);
  virtual void DidAppendChild(const ContainerNode& parent, const Node& child);
  virtual void DidChangeInlineStyle(const ElementNode& element,
                                    const css::Style* old_style);
  virtual void DidInsertBefore(const ContainerNode& parent,
                               const Node& child,
                               const Node& ref_child);
  virtual void DidRemoveChild(const ContainerNode& parent, const Node& child);
  virtual void DidRemoveClass(const ElementNode& element,
                              const base::string16& old_name);
  virtual void DidReplaceChild(const ContainerNode& parent,
                               const Node& new_child,
                               const Node& old_child);
  virtual void DidSetShapeData(const Shape& shape,
                               const ShapeData& new_data,
                               const ShapeData& old_data);
  virtual void DidSetTextData(const Text& text,
                              const base::string16& new_data,
                              const base::string16& old_data);
  virtual void WillRemoveChild(const ContainerNode& parent, const Node& child);

 protected:
  DocumentObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(DocumentObserver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_OBSERVER_H_
