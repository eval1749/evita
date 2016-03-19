// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_mutation_record.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationRecord
//
TextMutationRecord::TextMutationRecord(const base::string16& type,
                                       TextDocument* document,
                                       text::Offset mutation_start,
                                       text::Offset mutation_end,
                                       text::Offset document_end)
    : document_(document),
      document_end_(document_end),
      mutation_end_(mutation_end),
      mutation_start_(mutation_start),
      type_(type) {
  DCHECK_LE(mutation_start_, mutation_end_);
  DCHECK_LE(mutation_end_, document_end_);
}

TextMutationRecord::~TextMutationRecord() {}

}  // namespace dom
