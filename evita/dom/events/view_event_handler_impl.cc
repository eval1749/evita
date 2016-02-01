// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_handler_impl.h"

#include "base/strings/utf_string_conversions.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/bindings/v8_glue_UiEventInit.h"
#include "evita/dom/bindings/v8_glue_WindowEventInit.h"
#include "evita/dom/events/composition_event.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/events/view_event_target_set.h"
#include "evita/dom/events/wheel_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/text_composition_event.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/v8_strings.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_set.h"
#include "evita/gc/local.h"
#include "evita/text/buffer.h"
#include "evita/v8_glue/runner.h"

namespace dom {

namespace {

base::string16 V8ToString(v8::Local<v8::Value> value) {
  if (value.IsEmpty())
    return L"(empty)";
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

ViewEventTarget* FromEventTargetId(domapi::EventTargetId event_target_id) {
  auto const target = ViewEventTargetSet::instance()->Find(event_target_id);
  if (!target)
    DVLOG(0) << "No such event target " << event_target_id;
  return target;
}

Window* FromWindowId(domapi::WindowId window_id) {
  DCHECK_NE(domapi::kInvalidWindowId, window_id);
  auto const window = WindowSet::instance()->Find(window_id);
  if (!window)
    DVLOG(0) << "No such window " << window_id << ".";
  return window;
}

v8::Local<v8::Value> GetOpenFileHandler(v8_glue::Runner* runner,
                                        domapi::WindowId window_id) {
  auto const isolate = runner->isolate();
  if (window_id == domapi::kInvalidWindowId)
    return runner->global()->Get(gin::StringToV8(isolate, "Editor"));

  auto const window = FromWindowId(window_id);
  if (!window)
    return v8::Local<v8::Value>();
  return window->GetWrapper(isolate);
}

ViewEventTarget* MaybeEventTarget(domapi::EventTargetId event_target_id) {
  if (event_target_id == domapi::kInvalidEventTargetId)
    return nullptr;
  return FromEventTargetId(event_target_id);
}

// Returns existing or new |EditorWindow| for drag-and-drop.
Window* NewOrFromWindowId(domapi::WindowId window_id) {
  if (window_id != domapi::kInvalidWindowId)
    return FromWindowId(window_id);
  // TODO(eval1749): We should not constructor |EditorWindow| here.
  return new EditorWindow(ScriptHost::instance());
}

}  // namespace

ViewEventHandlerImpl::ViewEventHandlerImpl(ScriptHost* host) : host_(host) {}

ViewEventHandlerImpl::~ViewEventHandlerImpl() {}

void ViewEventHandlerImpl::DispatchEventWithInLock(EventTarget* event_target,
                                                   Event* event) {
  TRACE_EVENT1("script", "ViewEventHandlerImpl::DispatchEventWithInLock",
               "type", base::UTF16ToASCII(event->type()));
  auto const runner = host_->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  v8::TryCatch try_catch(runner->isolate());
  try_catch.SetVerbose(true);
  DOM_AUTO_LOCK_SCOPE();
  if (event_target->DispatchEvent(event))
    host_->CallClassEventHandler(event_target, event);
  runner->HandleTryCatch(try_catch);
}

// domapi::ViewEventHandler
void ViewEventHandlerImpl::DidActivateWindow(domapi::WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidActivateWindow();
}

void ViewEventHandlerImpl::DidBeginFrame(const base::TimeTicks& deadline) {
  NOTREACHED();
}

void ViewEventHandlerImpl::DidChangeWindowBounds(domapi::WindowId window_id,
                                                 int left,
                                                 int top,
                                                 int right,
                                                 int bottom) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidChangeBounds(left, top, right, bottom);
}

void ViewEventHandlerImpl::DidChangeWindowVisibility(
    domapi::WindowId window_id,
    domapi::Visibility visibility) {
  auto const target = FromEventTargetId(window_id);
  if (!target)
    return;
  if (const auto window = target->as<Window>()) {
    if (visibility == domapi::Visibility::Visible)
      window->DidShowWindow();
    else
      window->DidHideWindow();
  }
  DispatchEventWithInLock(
      target,
      new UiEvent(visibility == domapi::Visibility::Visible ? L"show" : L"hide",
                  UiEventInit()));
}

void ViewEventHandlerImpl::DidDestroyWindow(domapi::WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidDestroyWindow();
}

void ViewEventHandlerImpl::DidDropWidget(domapi::WindowId source_id,
                                         domapi::WindowId target_id) {
  auto const source_window = FromWindowId(source_id);
  if (!source_window)
    return;
  auto const target_window = NewOrFromWindowId(target_id);
  if (!target_window)
    return;
  WindowEventInit init_dict;
  init_dict.set_bubbles(false);
  init_dict.set_cancelable(false);
  init_dict.set_source_window(source_window);
  DispatchEventWithInLock(target_window,
                          new WindowEvent(L"dropwindow", init_dict));
}

void ViewEventHandlerImpl::DidEnterViewIdle(const base::TimeTicks& deadline) {
  NOTREACHED();
}

void ViewEventHandlerImpl::DidExitViewIdle() {
  NOTREACHED();
}

void ViewEventHandlerImpl::DidRealizeWidget(domapi::WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidRealizeWindow();
}

void ViewEventHandlerImpl::DispatchFocusEvent(
    const domapi::FocusEvent& api_event) {
  auto const target = FromEventTargetId(api_event.target_id);
  if (!target)
    return;
  if (auto const window = target->as<Window>()) {
    if (api_event.event_type == domapi::EventType::Focus)
      window->DidSetFocus();
    else
      window->DidKillFocus();
  }
  const auto& related_target = MaybeEventTarget(api_event.related_target_id);
  const auto& target_and_event =
      target->TranslateFocusEvent(api_event, related_target);
  DispatchEventWithInLock(target_and_event.first, target_and_event.second);
}

void ViewEventHandlerImpl::DispatchKeyboardEvent(
    const domapi::KeyboardEvent& api_event) {
  TRACE_EVENT_WITH_FLOW0("input", "ViewEventHandlerImpl::DispatchKeyboardEvent",
                         api_event.event_id, TRACE_EVENT_FLAG_FLOW_IN);
  auto const window = FromEventTargetId(api_event.target_id);
  if (window) {
    const auto& target_and_event = window->TranslateKeyboardEvent(api_event);
    DispatchEventWithInLock(target_and_event.first, target_and_event.second);
  }
  TRACE_EVENT_ASYNC_END0("input", "KeyEvent", api_event.event_id);
}

void ViewEventHandlerImpl::DispatchMouseEvent(
    const domapi::MouseEvent& api_event) {
  TRACE_EVENT_WITH_FLOW0("input", "ViewEventHandlerImpl::DispatchMouseEvent",
                         api_event.event_id, TRACE_EVENT_FLAG_FLOW_IN);
  auto const window = FromEventTargetId(api_event.target_id);
  if (window) {
    const auto& target_and_event = window->TranslateMouseEvent(api_event);
    DispatchEventWithInLock(target_and_event.first, target_and_event.second);
  }
  TRACE_EVENT_ASYNC_END0("input", "MouseEvent", api_event.event_id);
}

void ViewEventHandlerImpl::DispatchTextCompositionEvent(
    const domapi::TextCompositionEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEventWithInLock(window, new CompositionEvent(api_event));
}

void ViewEventHandlerImpl::DispatchWheelEvent(
    const domapi::WheelEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEventWithInLock(window, new WheelEvent(api_event));
}

void ViewEventHandlerImpl::OpenFile(domapi::WindowId window_id,
                                    const base::string16& file_name) {
  auto const runner = host_->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const isolate = runner->isolate();
  auto const js_handler = GetOpenFileHandler(runner, window_id);
  if (js_handler.IsEmpty() || !js_handler->IsObject())
    return;
  auto const open_file =
      js_handler->ToObject()->Get(gin::StringToV8(isolate, "open"));
  if (!open_file->IsFunction()) {
    DVLOG(0) << "OpenFile: window doesn't have callable open property.";
    return;
  }
  v8::Local<v8::Value> js_file_name = gin::StringToV8(isolate, file_name);
  DOM_AUTO_LOCK_SCOPE();
  runner->CallAsFunction(open_file, js_handler, js_file_name);
}

void ViewEventHandlerImpl::ProcessCommandLine(
    const base::string16& working_directory,
    const std::vector<base::string16>& args) {
  auto const runner = host_->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const isolate = runner->isolate();
  auto const editor = runner->global()->Get(gin::StringToV8(isolate, "Editor"));
  auto const process =
      editor->ToObject()->Get(gin::StringToV8(isolate, "processCommandLine"));
  if (!process->IsFunction()) {
    DVLOG(0) << "No Editor.processCommandLine";
    return;
  }
  DOM_AUTO_LOCK_SCOPE();
  runner->CallAsFunction(
      process, editor, gin::ConvertToV8(isolate, working_directory),
      gin::ConvertToV8(runner->context(), args).ToLocalChecked());
}

void ViewEventHandlerImpl::QueryClose(domapi::WindowId window_id) {
  auto const window = WindowSet::instance()->Find(window_id);
  if (!window)
    return;
  UiEventInit init_dict;
  init_dict.set_bubbles(false);
  init_dict.set_cancelable(true);
  init_dict.set_view(window);
  DispatchEventWithInLock(window, new UiEvent(L"queryclose", init_dict));
}

void ViewEventHandlerImpl::RunCallback(const base::Closure& callback) {
  TRACE_EVENT0("script", "ViewEventHandlerImpl::RunCallback");
  DOM_AUTO_LOCK_SCOPE();
  callback.Run();
}

void ViewEventHandlerImpl::WillDestroyViewHost() {
  host_->WillDestroyViewHost();
}

}  // namespace dom
