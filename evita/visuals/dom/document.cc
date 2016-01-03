// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Document::LockScope
//
Document::LockScope::LockScope(const Document& document) : document_(document) {
  document_.Lock();
}

Document::LockScope::~LockScope() {
  document_.Unlock();
}

//////////////////////////////////////////////////////////////////////
//
// Document
//
Document::Document() : ContainerNode(this, L"#document") {}
Document::~Document() {}

void Document::AddObserver(DocumentObserver* observer) const {
  const_cast<Document*>(this)->observers_.AddObserver(observer);
}

Node* Document::GetNodeById(const base::StringPiece16& id) const {
  const auto& it = id_map_.find(id.as_string());
  if (it == id_map_.end())
    return nullptr;
  // TODO(eval1749): We should return a first node in tree order.
  return it->second;
}

void Document::Lock() const {
  ++lock_count_;
}

void Document::RegisterNodeIdIfNeeded(const Node& node) {
  DCHECK(!is_locked());
  if (node.id().empty())
    return;
  const auto& result =
      id_map_.insert(std::make_pair(node.id(), const_cast<Node*>(&node)));
  DCHECK(result.second) << "id_map_ already has " << node;
}

void Document::RemoveObserver(DocumentObserver* observer) const {
  const_cast<Document*>(this)->observers_.RemoveObserver(observer);
}

void Document::Unlock() const {
  DCHECK_GT(lock_count_, 0);
  --lock_count_;
}

void Document::UnregisterNodeIdIfNeeded(const Node& node) {
  DCHECK(!is_locked());
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
