// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/view_event_handler_impl.h"

#include "evita/bindings/FocusEventInit.h"
#include "evita/bindings/UiEventInit.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/events/view_event_target_set.h"
#include "evita/dom/events/wheel_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/events/window_event_init.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_set.h"
#include "evita/gc/local.h"
#include "evita/text/buffer.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"

namespace dom {

namespace {

base::string16 V8ToString(v8::Handle<v8::Value> value) {
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

Window* FromWindowId(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  auto const window = WindowSet::instance()->Find(window_id);
  if (!window)
    DVLOG(0) << "No such window " << window_id << ".";
  return window;
}

ViewEventTarget* MaybeEventTarget(domapi::EventTargetId event_target_id) {
  if (event_target_id == domapi::kInvalidEventTargetId)
    return nullptr;
  return FromEventTargetId(event_target_id);
}

Window* NewOrFromWindowId(WindowId window_id) {
  return window_id == kInvalidWindowId ? new EditorWindow() :
      FromWindowId(window_id);
}

// Note: The constructor returned by v8::Object::GetConstructor() doesn't
// have properties defined in JavaScript.
v8::Handle<v8::Object> GetClassObject(v8::Isolate* isolate,
                                      v8::Handle<v8::Object> object) {
  auto const name = object->GetConstructorName();
  auto const value = isolate->GetCurrentContext()->Global()->Get(name);
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "No such class " << V8ToString(name) << ".";
    return v8::Handle<v8::Object>();
  }
  return value->ToObject();
}

v8::Handle<v8::Object> ToMethodObject(v8::Isolate* isolate,
                                      v8::Handle<v8::Object> js_class,
                                      v8::Eternal<v8::String> method_name) {
  auto const value = js_class->Get(method_name.Get(isolate));
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "Object " << V8ToString(js_class) << " has no method '" <<
        V8ToString(method_name.Get(isolate)) << "', it has " <<
        V8ToString(js_class->GetPropertyNames()) << ".";
    return v8::Handle<v8::Object>();
  }
  return value->ToObject();
}
}  // namespace

ViewEventHandlerImpl::ViewEventHandlerImpl(ScriptHost* controller)
    : controller_(controller) {
}

ViewEventHandlerImpl::~ViewEventHandlerImpl() {
}

void ViewEventHandlerImpl::DispatchEvent(EventTarget* event_target,
                                         Event* event) {
  if (!event_target->DispatchEvent(event))
    return;

  // Call |handleEvent| function in the class of event target.
  auto const runner = controller_->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);

  auto const js_target = event_target->GetWrapper(isolate);
  auto const js_class = GetClassObject(isolate, js_target);
  if (js_class.IsEmpty())
    return;

  auto const js_method = ToMethodObject(isolate, js_class,
                                        v8Strings::handleEvent);
  if (js_method.IsEmpty())
    return;

  auto const js_event = event->GetWrapper(isolate);
  runner->Call(js_method, js_target, js_event);
}

void ViewEventHandlerImpl::DispatchEventWithInLock(
    EventTarget* event_target, Event* event) {
  DOM_AUTO_LOCK_SCOPE();
  DispatchEvent(event_target, event);
}

// domapi::ViewEventHandler
void ViewEventHandlerImpl::DidDestroyWidget(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidDestroyWindow();
}

void ViewEventHandlerImpl::DidDropWidget(WindowId source_id,
                                         WindowId target_id) {
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

void ViewEventHandlerImpl::DidRealizeWidget(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidRealizeWindow();
}

void ViewEventHandlerImpl::DidResizeWidget(WindowId window_id, int left, int top,
                                   int right, int bottom) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidResize(left, top, right, bottom);
}

void ViewEventHandlerImpl::DidStartViewHost() {
  controller_->DidStartViewHost();
}

void ViewEventHandlerImpl::DispatchFocusEvent(
    const domapi::FocusEvent& api_event) {
  auto const target = FromEventTargetId(api_event.target_id);
  if (!target)
    return;
  if (api_event.event_type == domapi::EventType::Focus) {
    if (auto const window = target->as<Window>())
      window->DidSetFocus();
    else if (auto const form_control = target->as<FormControl>())
      form_control->DidSetFocus();
  }
  FocusEventInit event_init;
  event_init.set_related_target(MaybeEventTarget(api_event.related_target_id));
  DispatchEventWithInLock(target, new FocusEvent(
      api_event.event_type == domapi::EventType::Blur ? L"blur" : L"focus",
      event_init));
}

void ViewEventHandlerImpl::DispatchKeyboardEvent(
    const domapi::KeyboardEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEventWithInLock(window, new KeyboardEvent(api_event));
}

void ViewEventHandlerImpl::DispatchMouseEvent(
    const domapi::MouseEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEventWithInLock(window, new MouseEvent(api_event));
}

void ViewEventHandlerImpl::DispatchViewIdleEvent(int hint) {
  {
    DOM_AUTO_LOCK_SCOPE();
    auto const runner = ScriptHost::instance()->runner();
    auto const isolate = runner->isolate();
    v8_glue::Runner::Scope runner_scope(runner);
    auto const window_class = runner->global()->
        Get(v8Strings::Window.Get(isolate));
    auto const focus_window = window_class->ToObject()->
        Get(v8Strings::focus.Get(isolate));
    auto event_target = static_cast<EventTarget*>(nullptr);
    if (gin::ConvertFromV8(isolate, focus_window, &event_target)) {
      UiEventInit init_dict;
      init_dict.set_bubbles(true);
      init_dict.set_cancelable(true);
      init_dict.set_detail(hint);
      DispatchEvent(event_target, new UiEvent(L"idle", init_dict));
    }
    v8::V8::RunMicrotasks(isolate);
  }
  // TODO(yosi) We should ask view host to stop dispatching idle event, if
  // idle event handler throws an exception.
  ScriptHost::instance()->view_delegate()->DidHandleViewIdelEvent(hint);
}

void ViewEventHandlerImpl::DispatchWheelEvent(
    const domapi::WheelEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEventWithInLock(window, new WheelEvent(api_event));
}

void ViewEventHandlerImpl::OpenFile(WindowId window_id,
                            const base::string16& file_name){
  controller_->OpenFile(window_id, file_name);
}

void ViewEventHandlerImpl::QueryClose(WindowId window_id) {
  auto const window = WindowSet::instance()->Find(window_id);
  if (!window)
    return;
  UiEventInit init_dict;
  init_dict.set_bubbles(false);
  init_dict.set_cancelable(true);
  init_dict.set_view(window);
  DispatchEventWithInLock(window, new UiEvent(L"queryclose", init_dict));
}

void ViewEventHandlerImpl::RunCallback(base::Closure callback) {
  DOM_AUTO_LOCK_SCOPE();
  callback.Run();
}

void ViewEventHandlerImpl::WillDestroyHost() {
  controller_->WillDestroyHost();
}

} // namespace dom
