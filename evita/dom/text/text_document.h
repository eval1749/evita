// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TEXT_TEXT_DOCUMENT_H_
#define EVITA_DOM_TEXT_TEXT_DOCUMENT_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/dom/events/event_target.h"
#include "evita/gc/member.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/scriptable.h"

namespace text {
class Buffer;
struct LineAndColumn;
class Offset;
}

namespace dom {

class ExceptionState;
class RegularExpression;
class TextRange;

//////////////////////////////////////////////////////////////////////
//
// TextDocument
//
class TextDocument final : public ginx::Scriptable<TextDocument, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(TextDocument);

 public:
  ~TextDocument() final;

  const text::Buffer* buffer() const { return buffer_.get(); }
  text::Buffer* buffer() { return buffer_.get(); }
  base::char16 charCodeAt(text::Offset position,
                          ExceptionState* exception_state) const;
  int length() const;
  bool read_only() const;
  int revision() const;
  void set_read_only(bool read_only) const;
  // Returns spelling at |offset|.
  const base::string16& spelling_at(text::Offset offset,
                                    ExceptionState* exception_state) const;
  // Returns syntax at |offset|.
  const base::string16& syntax_at(text::Offset offset,
                                  ExceptionState* exception_state) const;
  v8::Local<v8::Object> style_at(text::Offset position,
                                 ExceptionState* exception_state) const;

  bool CheckCanChange(ExceptionState* exception_state) const;
  void ClearUndo();
  void EndUndoGroup(const base::string16& name);
  text::LineAndColumn GetLineAndColumn(text::Offset offset,
                                       ExceptionState* exception_state) const;
  bool IsValidPosition(text::Offset offset,
                       ExceptionState* exception_state) const;
  bool IsValidRange(text::Offset start,
                    text::Offset end,
                    ExceptionState* exception_state) const;
  v8::Local<v8::Value> Match(RegularExpression* regexp,
                             text::Offset start,
                             text::Offset end);
  text::Offset Redo(text::Offset position);
  void SetSpelling(text::Offset start,
                   text::Offset end,
                   int spelling,
                   ExceptionState* exception_state);
  void SetSyntax(text::Offset start,
                 text::Offset end,
                 const base::string16& syntax,
                 ExceptionState* exception_state);
  base::string16 Slice(int start, int end);
  base::string16 Slice(int start);
  void StartUndoGroup(const base::string16& name);
  text::Offset Undo(text::Offset position);
  text::Offset ValidateOffset(int offsetLike,
                              ExceptionState* exception_state) const;

  // Implementation of TextDocument static method
  static TextDocument* NewTextDocument();

 private:
  TextDocument();

  std::unique_ptr<text::Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(TextDocument);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_DOCUMENT_H_
