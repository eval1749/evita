// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DOCUMENT_H_
#define EVITA_VISUALS_DOM_DOCUMENT_H_

#include <map>

#include "base/observer_list.h"
#include "evita/visuals/dom/container_node.h"

namespace visuals {

class DocumentObserver;

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document final : public ContainerNode {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Document, ContainerNode);

 public:
  class LockScope final {
   public:
    explicit LockScope(const Document& document);
    ~LockScope();

   private:
    const Document& document_;

    DISALLOW_COPY_AND_ASSIGN(LockScope);
  };

  Document();
  ~Document() final;

  bool is_locked() const { return lock_count_ > 0; }
  int version() const { return version_; }

  void AddObserver(DocumentObserver* observer) const;

  ElementNode* GetElementById(const base::StringPiece16& id) const;
  void Lock() const;
  void Unlock() const;

  void RemoveObserver(DocumentObserver* observer) const;

 private:
  void RegisterElementIdIfNeeded(const ElementNode& element);
  void UnregisterElementIdIfNeeded(const ElementNode& element);

  // gc::Visitable
  void Accept(gc::Visitor* visitor) final;

  base::ObserverList<DocumentObserver> observers_;
  std::map<base::string16, ElementNode*> id_map_;
  mutable int lock_count_ = 0;
  int version_ = 0;

  DISALLOW_COPY_AND_ASSIGN(Document);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DOCUMENT_H_
