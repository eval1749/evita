// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_
#define EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/ginx/scriptable.h"
#include "evita/text/offset.h"

namespace dom {

class TextDocument;

namespace bindings {
class TextMutationRecordClass;
}

class TextMutationRecord final : public ginx::Scriptable<TextMutationRecord> {
  DECLARE_SCRIPTABLE_OBJECT(TextMutationRecord);

 public:
  TextMutationRecord(const base::string16& type,
                     TextDocument* document,
                     text::Offset mutation_start,
                     text::Offset mutation_end,
                     text::Offset document_end);
  ~TextMutationRecord() final;

 private:
  friend class bindings::TextMutationRecordClass;

  TextDocument* document() const { return document_.get(); }

  // For IDL
  // TODO(eval1749): We should get rid of |deprecated_offset()| from
  // |TextMutationRecord|.
  int deprecated_offset() const { return mutation_start_.value(); }
  int document_end() const { return document_end_.value(); }
  int end() const { return mutation_end_.value(); }
  int start() const { return mutation_start_.value(); }
  const base::string16& type() const { return type_; }

  gc::Member<TextDocument> document_;
  // A end offset of |document_| at start of recording.
  const text::Offset document_end_;
  // An end offset of mutation as start of recording.
  const text::Offset mutation_end_;
  // A start offset of mutation.
  const text::Offset mutation_start_;
  base::string16 type_;

  DISALLOW_COPY_AND_ASSIGN(TextMutationRecord);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_
