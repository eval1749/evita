// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_document_set_h)
#define INCLUDE_evita_dom_document_set_h

#include <unordered_map>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class Document;

class DocumentSet : public common::Singleton<DocumentSet> {
  friend class common::Singleton<DocumentSet>;

  private: std::unordered_map<base::string16, Document*> map_;

  private: DocumentSet();
  public: ~DocumentSet();

  private: std::vector<Document*> list() const;

  public: Document* Find(const base::string16 name) const;
  public: base::string16 MakeUniqueName(const base::string16& name);
  public: void Register(Document* document);
  public: void ResetForTesting();
  public: static std::vector<Document*> StaticList();
  public: static v8_glue::Nullable<Document> StaticFind(
      const base::string16& name);
  public: static void StaticRemove(Document* document);
  public: void Unregister(Document* document);

  DISALLOW_COPY_AND_ASSIGN(DocumentSet);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_document_set_h)
