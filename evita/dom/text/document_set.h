// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_DOCUMENT_SET_H_
#define EVITA_DOM_TEXT_DOCUMENT_SET_H_

#include <unordered_map>
#include <vector>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/v8.h"

namespace dom {

class Document;

class DocumentSet final : public common::Singleton<DocumentSet> {
 public:
  ~DocumentSet() final;

  std::vector<Document*> list() const;

  void AddObserver(v8::Handle<v8::Function> callback);
  Document* Find(const base::string16 name) const;
  base::string16 MakeUniqueName(const base::string16& name);
  void Register(Document* document);
  void RemoveObserver(v8::Handle<v8::Function> callback);
  void RenameDocument(Document* document, const base::string16& new_name);
  void ResetForTesting();
  void Unregister(Document* document);

 private:
  friend class common::Singleton<DocumentSet>;
  class Observer;

  DocumentSet();

  void NotifyObserverWithInLock(const base::string16& type, Document* document);
  void ScheduleNotifyObserver(const base::string16& type, Document* document);

  std::unordered_map<base::string16, Document*> map_;
  std::vector<Observer*> observers_;

  DISALLOW_COPY_AND_ASSIGN(DocumentSet);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_DOCUMENT_SET_H_
