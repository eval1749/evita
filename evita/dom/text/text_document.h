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
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Buffer;
struct LineAndColumn;
class Offset;
}

namespace dom {

class Range;
class RegularExpression;

//////////////////////////////////////////////////////////////////////
//
// TextDocument
//
class TextDocument final
    : public v8_glue::Scriptable<TextDocument, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(TextDocument);

 public:
  ~TextDocument() final;

  const text::Buffer* buffer() const { return buffer_.get(); }
  text::Buffer* buffer() { return buffer_.get(); }
  base::char16 charCodeAt(text::Offset position) const;
  int length() const;
  bool read_only() const;
  int revision() const;
  void set_read_only(bool read_only) const;
  // Returns spelling at |offset|.
  const base::string16& spelling_at(text::Offset offset) const;
  // Returns syntax at |offset|.
  const base::string16& syntax_at(text::Offset offset) const;
  v8::Handle<v8::Object> style_at(text::Offset position) const;

  bool CheckCanChange() const;
  void ClearUndo();
  void EndUndoGroup(const base::string16& name);
  text::LineAndColumn GetLineAndColumn(text::Offset offset) const;
  bool IsValidPosition(text::Offset offset) const;
  bool IsValidRange(text::Offset start, text::Offset end) const;
  v8::Handle<v8::Value> Match(RegularExpression* regexp,
                              text::Offset start,
                              text::Offset end);
  text::Offset Redo(text::Offset position);
  void RenameTo(const base::string16& new_name);
  void SetSpelling(text::Offset start, text::Offset end, int spelling);
  void SetSyntax(text::Offset start,
                 text::Offset end,
                 const base::string16& syntax);
  base::string16 Slice(int start, int end);
  base::string16 Slice(int start);
  void StartUndoGroup(const base::string16& name);
  text::Offset Undo(text::Offset position);
  text::Offset ValidateOffset(int offsetLike) const;

  // Implementation of TextDocument static method
  static TextDocument* NewTextDocument();

 private:
  TextDocument();

  std::unique_ptr<text::Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(TextDocument);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_TEXT_DOCUMENT_H_
