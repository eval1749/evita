// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document.h"

#include "base/logging.h"
#include "evita/visuals/dom/element.h"

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

ElementNode* Document::GetElementById(base::StringPiece16 id_value) const {
  AtomicString id(id_value);
  const auto& it = id_map_.find(id);
  if (it == id_map_.end())
    return nullptr;
  // TODO(eval1749): We should return a first node in tree order.
  return it->second;
}

void Document::Lock() const {
  ++lock_count_;
}

void Document::RegisterElementIdIfNeeded(const ElementNode& element) {
  DCHECK(!is_locked());
  if (element.id().empty())
    return;
  const auto& result = id_map_.insert(
      std::make_pair(element.id(), const_cast<ElementNode*>(&element)));
  DCHECK(result.second) << "id_map_ already has " << element;
}

void Document::RemoveObserver(DocumentObserver* observer) const {
  const_cast<Document*>(this)->observers_.RemoveObserver(observer);
}

void Document::Unlock() const {
  DCHECK_GT(lock_count_, 0);
  --lock_count_;
}

void Document::UnregisterElementIdIfNeeded(const ElementNode& element) {
  DCHECK(!is_locked());
  if (element.id().empty())
    return;
  const auto& start = id_map_.find(element.id());
  DCHECK(start != id_map_.end()) << "id_map_ should have " << element;
  for (auto it = start; it != id_map_.end() && it->first == element.id();
       ++it) {
    if (it->second == &element) {
      id_map_.erase(it);
      return;
    }
  }
  DVLOG(ERROR) << "id_map_ should have " << element;
}

// gc::Visitable
void Document::Accept(gc::Visitor* visitor) {
  for (const auto& entry : id_map_)
    visitor->Visit(entry.second);
}

}  // namespace visuals
