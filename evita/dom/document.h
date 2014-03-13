// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_document_h)
#define INCLUDE_evita_dom_document_h

#include <memory>
#include <unordered_set>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class Buffer;
class Mode;
class Range;
class RegExp;

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document : public v8_glue::Scriptable<Document> {
  DECLARE_SCRIPTABLE_OBJECT(Document);

  private: std::unique_ptr<Buffer> buffer_;
  private: gc::Member<Mode> mode_;
  private: std::unordered_set<Range*> ranges_;
  // TODO(yosi) When we set |properties| with |v8::Object::Set()|, it doesn't
  // appeared in JavaScript. I'm not sure why. So, we hold |properties| in
  // C++.
  private: v8_glue::ScopedPersistent<v8::Object> properties_;

  public: explicit Document(const base::string16& name, Mode* mode);
  public: virtual ~Document();

  public: const Buffer* buffer() const { return buffer_.get(); }
  public: Buffer* buffer() { return buffer_.get(); }
  public: base::char16 charCodeAt(text::Posn position) const;
  public: const base::string16& filename() const;
  public: void set_filename(const base::string16& filename);
  public: base::Time last_write_time() const;
  public: void set_last_write_time(base::Time last_write_time);
  public: text::Posn length() const;
  public: Mode* mode() const { return mode_.get(); }
  public: void set_mode(Mode* mode);
  public: bool modified() const;
  public: const base::string16& name() const;
  public: v8::Handle<v8::Object> properties() const;
  public: bool read_only() const;
  public: void set_read_only(bool read_only) const;
  public: int spelling_at(text::Posn offset) const;
  public: v8::Handle<v8::Object> style_at(text::Posn position) const;

  public: bool CheckCanChange() const;
  public: void DidCreateRange(Range* range);
  public: void DidDestroyRange(Range* range);
  public: void DoColor(int hint);
  public: void EndUndoGroup(const base::string16& name);
  public: static Document* Find(const base::string16& name);
  public: bool IsValidPosition(text::Posn position) const;
  public: void Load(const base::string16& filename,
                    v8::Handle<v8::Function> callback);
  public: v8::Handle<v8::Value> Match(RegExp* regexp, int start, int end);
  public: Posn Redo(Posn position);
  public: void RenameTo(const base::string16& new_name);
  public: static void ResetForTesting();
  public: void Save(const base::string16& filename,
                    v8::Handle<v8::Function> callback);
  public: base::string16 Slice(int start, v8_glue::Optional<int> opt_end);
  public: void StartUndoGroup(const base::string16& name);
  public: Posn Undo(Posn position);

  DISALLOW_COPY_AND_ASSIGN(Document);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_document_h)
