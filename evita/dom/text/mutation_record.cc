// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/mutation_record.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// MutationRecord
//
MutationRecord::MutationRecord(const base::string16& type,
                               TextDocument* document,
                               text::Offset offset)
    : document_(document), offset_(offset), type_(type) {}

MutationRecord::~MutationRecord() {}

}  // namespace dom
