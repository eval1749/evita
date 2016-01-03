// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Document
//
Document::Document() : ContainerNode(this, L"#document") {}
Document::~Document() {}

Node* Document::GetNodeById(const base::StringPiece16& id) const {
  const auto& it = id_map_.find(id.as_string());
  if (it == id_map_.end())
    return nullptr;
  // TODO(eval1749): We should return a first node in tree order.
  return it->second;
}

bool Document::InLayout() const {
  return lifecycle_.state() == DocumentLifecycle::State::InLayout;
}

bool Document::InPaint() const {
  return lifecycle_.state() == DocumentLifecycle::State::InPaint;
}

bool Document::IsLayoutClean() const {
  return lifecycle_.IsAtLeast(DocumentLifecycle::State::LayoutClean);
}

bool Document::IsPaintClean() const {
  return lifecycle_.IsAtLeast(DocumentLifecycle::State::PaintClean);
}

void Document::RegisterNodeIdIfNeeded(const Node& node) {
  DCHECK(lifecycle_.AllowsTreeMutaions()) << lifecycle_;
  if (node.id().empty())
    return;
  const auto& result =
      id_map_.insert(std::make_pair(node.id(), const_cast<Node*>(&node)));
  DCHECK(result.second) << "id_map_ already has " << node;
}

void Document::UnregisterNodeIdIfNeeded(const Node& node) {
  DCHECK(lifecycle_.AllowsTreeMutaions()) << lifecycle_;
  if (node.id().empty())
    return;
  const auto& start = id_map_.find(node.id());
  DCHECK(start != id_map_.end()) << "id_map_ should have " << node;
  for (auto it = start; it != id_map_.end() && it->first == node.id(); ++it) {
    if (it->second == &node) {
      id_map_.erase(it);
      return;
    }
  }
  DVLOG(ERROR) << "id_map_ should have " << node;
}

// gc::Visitable
void Document::Accept(gc::Visitor* visitor) {
  for (const auto& entry : id_map_)
    visitor->Visit(entry.second);
}

}  // namespace visuals
