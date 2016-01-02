// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_H_
#define EVITA_VISUALS_DOM_DOCUMENT_H_

#include <map>

#include "evita/visuals/geometry/float_size.h"
#include "evita/visuals/dom/container_node.h"
#include "evita/visuals/dom/document_lifecycle.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document final : public ContainerNode {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Document, ContainerNode);

 public:
  Document();
  ~Document() final;

  DocumentLifecycle* lifecycle() const { return &lifecycle_; }
  const FloatSize& viewport_size() const { return viewport_size_; }

  Node* GetNodeById(const base::StringPiece16& id) const;
  bool InLayout() const;
  bool InPaint() const;
  bool IsLayoutClean() const;
  bool IsPaintClean() const;

 private:
  void RegisterNodeIdIfNeeded(const Node& node);
  void UnregisterNodeIdIfNeeded(const Node& node);

  std::map<base::string16, Node*> id_map_;
  mutable DocumentLifecycle lifecycle_;
  FloatSize viewport_size_;

  DISALLOW_COPY_AND_ASSIGN(Document);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_H_
