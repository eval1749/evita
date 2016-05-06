// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_
#define EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/ginx/scriptable.h"
#include "evita/text/models/offset.h"

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
                     text::OffsetDelta delta,
                     text::OffsetDelta head_count,
                     text::OffsetDelta tail_count);
  ~TextMutationRecord() final;

 private:
  friend class bindings::TextMutationRecordClass;

  // For IDL
  int delta() const { return delta_.value(); }
  TextDocument* document() const { return document_.get(); }
  int head_count() const { return head_count_.value(); }
  int tail_count() const { return tail_count_.value(); }
  const base::string16& type() const { return type_; }

  gc::Member<TextDocument> document_;
  const text::OffsetDelta delta_;
  const text::OffsetDelta head_count_;
  const text::OffsetDelta tail_count_;
  base::string16 type_;

  DISALLOW_COPY_AND_ASSIGN(TextMutationRecord);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_MUTATION_RECORD_H_
