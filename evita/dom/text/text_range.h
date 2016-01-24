// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_RANGE_H_
#define EVITA_DOM_TEXT_TEXT_RANGE_H_

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/either.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Offset;
class Range;
}

namespace dom {

namespace bindings {
class TextRangeClass;
}

class ExceptionState;
class TextDocument;

//////////////////////////////////////////////////////////////////////
//
// TextRange
//
class TextRange final : public v8_glue::Scriptable<TextRange> {
  DECLARE_SCRIPTABLE_OBJECT(TextRange);

 public:
  TextRange(TextDocument* document, text::Offset start, text::Offset end);
  TextRange(TextDocument* document, text::Range* range);
  ~TextRange() final;

  bool collapsed() const;
  TextDocument* document() const { return document_.get(); }
  text::Offset end() const;
  text::Offset start() const;
  void set_end(int position);
  void set_start(int position);
  base::string16 text() const;
  void set_text(const base::string16& text);
  text::Range* text_range() const { return range_; }

  TextRange* CollapseTo(int position);
  TextRange* InsertBefore(const base::string16& text);
  static TextRange* NewTextRange(
      v8_glue::Either<TextDocument*, TextRange*> document_or_range);
  static TextRange* NewTextRange(
      v8_glue::Either<TextDocument*, TextRange*> document_or_range,
      int offset);
  static TextRange* NewTextRange(
      v8_glue::Either<TextDocument*, TextRange*> document_or_range,
      int start,
      int end);
  void SetSpelling(int spelling) const;
  void SetSyntax(const base::string16& syntax) const;

 private:
  friend class bindings::TextRangeClass;

  // bindings
  int end_value() const;
  void set_end_value(int value) { set_end(value); }
  int start_value() const;
  void set_start_value(int value) { set_start(value); }

  void SetStyle(v8::Local<v8::Object> style_dict,
                ExceptionState* exception_state) const;

  gc::Member<TextDocument> document_;
  // TODO(yosi): We should manage life time of text::Range.
  text::Range* range_;

  DISALLOW_COPY_AND_ASSIGN(TextRange);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_RANGE_H_
