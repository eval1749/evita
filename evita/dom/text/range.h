// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_RANGE_H_
#define EVITA_DOM_TEXT_RANGE_H_

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/either.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Offset;
class Range;
}

namespace dom {

class Document;

//////////////////////////////////////////////////////////////////////
//
// Range
//
class Range final : public v8_glue::Scriptable<Range> {
  DECLARE_SCRIPTABLE_OBJECT(Range);

 public:
  Range(Document* document, text::Offset start, text::Offset end);
  Range(Document* document, text::Range* range);
  ~Range() final;

  bool collapsed() const;
  Document* document() const { return document_.get(); }
  text::Offset end() const;
  text::Offset start() const;
  void set_end(int position);
  void set_start(int position);
  base::string16 text() const;
  void set_text(const base::string16& text);
  text::Range* text_range() const { return range_; }

  Range* CollapseTo(int position);
  Range* InsertBefore(const base::string16& text);
  static Range* NewRange(v8_glue::Either<Document*, Range*> document_or_range);
  static Range* NewRange(v8_glue::Either<Document*, Range*> document_or_range,
                         int offset);
  static Range* NewRange(v8_glue::Either<Document*, Range*> document_or_range,
                         int start,
                         int end);
  void SetSpelling(int spelling) const;
  void SetStyle(v8::Handle<v8::Object> style_dict) const;
  void SetSyntax(const base::string16& syntax) const;

 private:
  gc::Member<Document> document_;
  // TODO(yosi): We should manage life time of text::Range.
  text::Range* range_;

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_RANGE_H_
