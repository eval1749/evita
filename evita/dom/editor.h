// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EDITOR_H_
#define EVITA_DOM_EDITOR_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/windows/window.h"
#include "evita/ginx/scriptable.h"

namespace dom {

namespace bindings {
class EditorClass;
}

class IdleRequestOptions;
class Performance;

//////////////////////////////////////////////////////////////////////
//
// Editor
//
class Editor final : public ginx::Scriptable<Editor> {
  DECLARE_SCRIPTABLE_OBJECT(Editor)

 public:
  ~Editor() final;

 private:
  friend class bindings::EditorClass;

  Editor();

  static Performance* performance();
  static base::string16 v8_version();
  static base::string16 version();

  static v8::Local<v8::Promise> Editor::CheckSpelling(
      const base::string16& word_to_check);

  // |hint| is 1 to 1000: |v8::V8::IdelNotification(|hint|)| otherwise call
  // |v8::V8::LowMemoryNotification()|
  static bool CollectGarbage(int hint);
  static bool CollectGarbage();

  static v8::Local<v8::Promise> GetFileNameForLoad(
      Window* window,
      const base::string16& dir_path);
  static v8::Local<v8::Promise> GetFileNameForSave(
      Window* window,
      const base::string16& dir_path);
  static v8::Local<v8::Promise> GetMetrics(const base::string16& name);
  static v8::Local<v8::Promise> GetSpellingSuggestions(
      const base::string16& wrong_word);
  // Get global switch value.
  static domapi::SwitchValue GetSwitch(const base::string16& name);

  // Get global switch names.
  static std::vector<base::string16> GetSwitchNames();

  // Show message box
  static v8::Local<v8::Promise> MessageBox(Window* maybe_window,
                                           const base::string16& message,
                                           int flags,
                                           const base::string16& title);
  static v8::Local<v8::Promise> MessageBox(Window* maybe_window,
                                           const base::string16& message,
                                           int flags);

  // Run specified script
  static v8::Local<v8::Object> RunScript(const base::string16& script_text,
                                         const base::string16& file_name);
  static v8::Local<v8::Object> RunScript(const base::string16& script_text);

  // Set global switch value.
  static void SetSwitch(const base::string16& name,
                        const domapi::SwitchValue& new_value);

  static void SetTabData(Window* window, const domapi::TabData& tab_data);

  // Trace logging
  static void StartTraceLog(const base::string16& config);
  static void StopTraceLog(v8::Local<v8::Function> callback);

  // Request animation frame
  static void CancelAnimationFrame(int callback_id);
  static int RequestAnimationFrame(v8::Local<v8::Function> callback);

  // Request Idle Callback
  static int RequestIdleCallback(v8::Local<v8::Function> callback,
                                 const IdleRequestOptions& options);
  static int RequestIdleCallback(v8::Local<v8::Function> callback);
  static void CancelIdleCallback(int handle);

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<domapi::SwitchValue> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     domapi::SwitchValue* out);
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const domapi::SwitchValue& value);
};

template <>
struct Converter<domapi::TabData> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     domapi::TabData* out);
};
}  // namespace gin

#endif  // EVITA_DOM_EDITOR_H_
