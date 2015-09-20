// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_MUTATION_RECORD_H_
#define EVITA_DOM_TEXT_MUTATION_RECORD_H_

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

 public:
  MutationRecord(const base::string16& type,
                 Document* document,
                 text::Posn offset);
  ~MutationRecord() final;

 private:
  friend class bindings::MutationRecordClass;

  Document* document() const { return document_.get(); }
  text::Posn offset() const { return offset_; }
  const base::string16& type() const { return type_; }

  gc::Member<Document> document_;
  base::string16 type_;
  text::Posn offset_;

  DISALLOW_COPY_AND_ASSIGN(MutationRecord);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_MUTATION_RECORD_H_
