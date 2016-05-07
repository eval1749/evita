// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_RANGE_H_
#define EVITA_DOM_TEXT_TEXT_RANGE_H_

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/ginx/either.h"
#include "evita/ginx/scriptable.h"

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
class TextRange final : public ginx::Scriptable<TextRange> {
  DECLARE_SCRIPTABLE_OBJECT(TextRange);

 public:
  TextRange(TextDocument* document, text::Offset start, text::Offset end);
  TextRange(TextDocument* document, text::Range* range);
  ~TextRange() final;

  bool collapsed() const;
  TextDocument* document() const { return document_.get(); }
  text::Range* text_range() const { return range_; }

  TextRange* CollapseTo(int position, ExceptionState* exception_state);
  void InsertBefore(const base::string16& text,
                    ExceptionState* exception_state);
  static TextRange* NewTextRange(
      ginx::Either<TextDocument*, TextRange*> document_or_range,
      ExceptionState* exception_state);
  static TextRange* NewTextRange(
      ginx::Either<TextDocument*, TextRange*> document_or_range,
      int offset,
      ExceptionState* exception_state);
  static TextRange* NewTextRange(
      ginx::Either<TextDocument*, TextRange*> document_or_range,
      int start,
      int end,
      ExceptionState* exception_state);
  void SetSpelling(const base::string16& spelling,
                   ExceptionState* exception_state) const;
  void SetSyntax(const base::string16& syntax,
                 ExceptionState* exception_state) const;

 private:
  friend class bindings::TextRangeClass;

  // bindings
  int end() const;
  int start() const;
  base::string16 text() const;
  void set_end(int offsetLike, ExceptionState* exception_state);
  void set_start(int offsetLike, ExceptionState* exception_state);
  void set_text(const base::string16& text, ExceptionState* exception_state);

  gc::Member<TextDocument> document_;
  // TODO(yosi): We should manage life time of text::Range.
  text::Range* range_;

  DISALLOW_COPY_AND_ASSIGN(TextRange);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_RANGE_H_
