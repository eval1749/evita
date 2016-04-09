// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_mutation_record.h"

#include "evita/dom/text/text_document.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationRecord
//
TextMutationRecord::TextMutationRecord(const base::string16& type,
                                       TextDocument* document,
                                       text::OffsetDelta delta,
                                       text::OffsetDelta head_count,
                                       text::OffsetDelta tail_count)
    : delta_(delta),
      document_(document),
      head_count_(head_count),
      tail_count_(tail_count),
      type_(type) {
  DCHECK_LE(head_count_.value(), document_->length());
  DCHECK_LE(tail_count_.value(), document_->length());
}

TextMutationRecord::~TextMutationRecord() {}

}  // namespace dom
