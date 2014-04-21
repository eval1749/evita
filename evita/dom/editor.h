// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_h)
#define INCLUDE_evita_dom_editor_h

#include "base/strings/string16.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

namespace bindings {
class EditorClass;
}

class Editor : public v8_glue::Scriptable<Editor> {
  DECLARE_SCRIPTABLE_OBJECT(Editor)
  friend class bindings::EditorClass;

  private: Editor();
  public: virtual ~Editor();

  private: static base::string16 version();

  private: static v8::Handle<v8::Promise> Editor::CheckSpelling(
      const base::string16& word_to_check);
  private: static v8::Handle<v8::Promise> GetFileNameForLoad(
      Window* window, const base::string16& dir_path);
  private: static v8::Handle<v8::Promise> GetFileNameForSave(
      Window* window, const base::string16& dir_path);
  private: static base::string16 GetMetrics(const base::string16& name);
  private: static v8::Handle<v8::Promise> GetSpellingSuggestions(
      const base::string16& wrong_word);
  // Get global switch value.
  private: static domapi::SwitchValue GetSwitch(const base::string16& name);

  // Get global switch names.
  private: static std::vector<base::string16> GetSwitchNames();

  // Show message box
  private: static v8::Handle<v8::Promise> MessageBox(
      Window* maybe_window,
      const base::string16& message, int flags,
      const base::string16& title);
  private: static v8::Handle<v8::Promise> MessageBox(
      Window* maybe_window,
      const base::string16& message, int flags);

  // Run specified script
  private: static v8::Handle<v8::Object> RunScript(
      const base::string16& script_text,
      const base::string16& file_name);
  private: static v8::Handle<v8::Object> RunScript(
      const base::string16& script_text);

  // Set global switch value.
  private: static void SetSwitch(const base::string16& name,
                                 const domapi::SwitchValue& new_value);

  private: static void SetTabData(Window* window,
                                  const domapi::TabData& tab_data);

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<domapi::SwitchValue> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     domapi::SwitchValue* out);
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const domapi::SwitchValue& value);
};

template<>
struct Converter<domapi::TabData> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     domapi::TabData* out);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_editor_h)
