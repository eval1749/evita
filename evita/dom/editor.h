// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_h)
#define INCLUDE_evita_dom_editor_h

#include "base/strings/string16.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Editor : public v8_glue::Scriptable<Editor> {
  DECLARE_SCRIPTABLE_OBJECT(Editor)

  private: Editor();
  public: virtual ~Editor();

  public: static base::string16 version();

  public: static v8::Handle<v8::Promise> Editor::CheckSpelling(
      const base::string16& word_to_check);
  public: static v8::Handle<v8::Promise> GetFilenameForLoad(
      v8_glue::Nullable<Window> window, const base::string16& dir_path);
  public: static v8::Handle<v8::Promise> GetFilenameForSave(
      v8_glue::Nullable<Window> window, const base::string16& dir_path);
  public: static base::string16 GetMetrics(const base::string16& name);
  public: static v8::Handle<v8::Promise> GetSpellingSuggestions(
      const base::string16& wrong_word);
  public: static v8::Handle<v8::Promise> MessageBox(
      v8_glue::Nullable<Window> maybe_window,
      const base::string16& message, int flags,
      v8_glue::Optional<base::string16> title);
  public: static Editor* NewEditor();
  public: static v8::Handle<v8::Object> RunScript(
      const base::string16& script_text,
      v8_glue::Optional<base::string16> opt_file_name);
  public: static void SetTabData(Window* window,
                                 const domapi::TabData tab_data);

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<domapi::TabData> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     domapi::TabData* out);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_editor_h)
