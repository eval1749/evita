// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_document_h)
#define INCLUDE_evita_dom_document_h

#include <memory>
#include <unordered_set>

#include "base/strings/string16.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Buffer;
class Range;

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document : public v8_glue::Scriptable<Document> {
  private: std::unique_ptr<Buffer> buffer_;
  private: std::unordered_set<Range*> ranges_;

  public: explicit Document(base::string16 name);
  public: virtual ~Document();

  public: const Buffer* buffer() const { return buffer_.get(); }
  public: Buffer* buffer() { return buffer_.get(); }
  public: base::char16 charCodeAt(text::Posn position) const;
  public: text::Posn length() const;
  public: const base::string16& name() const;
  public: static v8_glue::WrapperInfo* static_wrapper_info();

  public: void DidCreateRange(Range* range);
  public: void DidDestroyRange(Range* range);
  public: static Range* GetOrCreateRange(text::Range* range);
  public: bool IsValidPosition(text::Posn position) const;
  public: void RenameTo(const base::string16& new_name);
  public: static void ResetForTesting();

  DISALLOW_COPY_AND_ASSIGN(Document);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_document_h)
