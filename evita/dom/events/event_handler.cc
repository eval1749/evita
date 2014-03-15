// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_handler.h"

#include "evita/dom/editor_window.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/focus_event_init.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/form_event_init.h"
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
#include "evita/dom/public/api_event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/window.h"
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

ViewEventTarget* FromEventTargetId(EventTargetId event_target_id) {
  auto const target = ViewEventTargetSet::instance()->Find(event_target_id);
  if (!target)
    DVLOG(0) << "No such event target " << event_target_id;
  return target;
}

Window* FromWindowId(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  auto const window = Window::FromWindowId(window_id);
  if (!window)
    DVLOG(0) << "No such window " << window_id << ".";
  return window;
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

EventHandler::EventHandler(ScriptController* controller)
    : controller_(controller) {
}

EventHandler::~EventHandler() {
}

// Call |handleEvent| function in the class of event target.
void EventHandler::DispatchEvent(EventTarget* event_target, Event* event) {
  DOM_AUTO_LOCK_SCOPE();
  if (!event_target->DispatchEvent(event))
    return;
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

  runner->Call(js_method, js_target, event->GetWrapper(isolate));
}

// domapi::ViewEventHandler
void EventHandler::AppendTextToBuffer(text::Buffer* buffer,
                                      const base::string16& text) {
  DOM_AUTO_LOCK_SCOPE();
  auto const readonly = buffer->IsReadOnly();
  if (readonly)
    buffer->SetReadOnly(false);
  buffer->InsertBefore(buffer->GetEnd(), text);
  if (readonly)
    buffer->SetReadOnly(true);
}

void EventHandler::DidDestroyWidget(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidDestroyWindow();
}

void EventHandler::DidDropWidget(WindowId source_id,
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
  DispatchEvent(target_window, new WindowEvent(L"dropwindow", init_dict));
}

void EventHandler::DidKillFocus(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  DispatchEvent(window, new FocusEvent(L"blur", FocusEventInit()));
}

void EventHandler::DidRealizeWidget(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidRealizeWindow();
}

void EventHandler::DidResizeWidget(WindowId window_id, int left, int top,
                                   int right, int bottom) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidResize(left, top, right, bottom);
}

void EventHandler::DidRequestFocus(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidRequestFocus();
  DispatchEvent(window, new FocusEvent(L"focus", FocusEventInit()));
}

void EventHandler::DidStartHost() {
  controller_->DidStartHost();
}

void EventHandler::DispatchFormEvent(const domapi::FormEvent& raw_event) {
  auto const form_id = raw_event.target_id;
  auto const target = FromEventTargetId(raw_event.target_id);
  if (!target)
    return;
  auto const form = target->as<Form>();
  if (!form) {
    DVLOG(0) << "Event target " << form_id << " isn't a From.";
    return;
  }
  auto const control = form->control(raw_event.control_id);
  if (!control) {
    DVLOG(0) <<  "Form " << form_id << " doesn't have control " <<
        raw_event.control_id;
  }
  FormEventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(false);
  init_dict.set_data(raw_event.data);
  DispatchEvent(control, new FormEvent(raw_event.type, init_dict));
}

void EventHandler::DispatchKeyboardEvent(
    const domapi::KeyboardEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEvent(window, new KeyboardEvent(api_event));
}

void EventHandler::DispatchMouseEvent(const domapi::MouseEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEvent(window, new MouseEvent(api_event));
}

void EventHandler::DispatchWheelEvent(
    const domapi::WheelEvent& api_event) {
  auto const window = FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  DispatchEvent(window, new WheelEvent(api_event));
}

void EventHandler::OpenFile(WindowId window_id,
                            const base::string16& filename){
  controller_->OpenFile(window_id, filename);
}

void EventHandler::QueryClose(WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window)
    return;
  UiEventInit init_dict;
  init_dict.set_bubbles(false);
  init_dict.set_cancelable(true);
  init_dict.set_view(window);
  DispatchEvent(window, new UiEvent(L"queryclose", init_dict));
}

void EventHandler::RunCallback(base::Closure callback) {
  DOM_AUTO_LOCK_SCOPE();
  callback.Run();
}

void EventHandler::WillDestroyHost() {
  controller_->WillDestroyHost();
}

} // namespace dom
