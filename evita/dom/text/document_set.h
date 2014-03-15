// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_document_set_h)
#define INCLUDE_evita_dom_document_set_h

#include <unordered_map>
#include <vector>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/v8.h"

namespace dom {

class Document;

class DocumentSet : public common::Singleton<DocumentSet> {
  friend class common::Singleton<DocumentSet>;
  private: class Observer;

  private: std::unordered_map<base::string16, Document*> map_;
  private: std::vector<Observer*> observers_;

  private: DocumentSet();
  public: ~DocumentSet();

  public: std::vector<Document*> list() const;

  public: void AddObserver(v8::Handle<v8::Function> callback);
  public: Document* Find(const base::string16 name) const;
  public: base::string16 MakeUniqueName(const base::string16& name);
  private: void NotifyObserverWithInLock(const base::string16& type,
                                         Document* document);
  public: void Register(Document* document);
  public: void RemoveObserver(v8::Handle<v8::Function> callback);
  public: void RenameDocument(Document* document,
                              const base::string16& new_name);
  public: void ResetForTesting();
  private: void ScheduleNotifyObserver(const base::string16& type,
                                       Document* document);
  public: void Unregister(Document* document);

  DISALLOW_COPY_AND_ASSIGN(DocumentSet);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_document_set_h)
