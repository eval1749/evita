// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_range_h)
#define INCLUDE_evita_dom_range_h

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/precomp.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Document;
class RegExp;

//////////////////////////////////////////////////////////////////////
//
// Range
//
class Range : public v8_glue::Scriptable<Range> {
  DECLARE_SCRIPTABLE_OBJECT(Range);

  private: gc::Member<Document> document_;
  // TODO(yosi): We should manage life time of text::Range.
  private: text::Range* range_;

  public: Range(Document* document, text::Posn start, text::Posn end);
  public: Range(Document* document, text::Range* range);
  public: virtual ~Range();

  public: bool collapsed() const;
  public: Document* document() const { return document_.get(); }
  public: int end() const;
  public: int start() const;
  public: void set_end(int position);
  public: void set_start(int position);
  public: base::string16 text() const;
  public: void set_text(const base::string16& text);
  public: text::Range* text_range() const { return range_; }

  public: Range* CollapseTo(Posn position);
  public: Range* InsertBefore(const base::string16& text);
  public: void SetSpelling(int spelling) const;
  public: void SetStyle(v8::Handle<v8::Object> style_dict) const;
  public: void SetSyntax(const base::string16& syntax) const;

  DISALLOW_COPY_AND_ASSIGN(Range);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_range_h)
