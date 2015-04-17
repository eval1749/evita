// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mutation_record_h)
#define INCLUDE_evita_dom_mutation_record_h

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/precomp.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Document;

namespace bindings {
class MutationRecordClass;
}

class MutationRecord final : public v8_glue::Scriptable<MutationRecord> {
  DECLARE_SCRIPTABLE_OBJECT(MutationRecord);
  friend class bindings::MutationRecordClass;

  private: gc::Member<Document> document_;
  private: base::string16 type_;
  private: text::Posn offset_;

  public: MutationRecord(const base::string16& type, Document* document,
                         text::Posn offset);
  public: virtual ~MutationRecord();

  private: Document* document() const { return document_.get(); }
  private: text::Posn offset() const { return offset_; }
  private: const base::string16& type() const { return type_; }

  DISALLOW_COPY_AND_ASSIGN(MutationRecord);
};

}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_mutation_record_h)
