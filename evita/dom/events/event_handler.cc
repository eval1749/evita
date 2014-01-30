// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_handler.h"

#include "evita/dom/editor_window.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/public/api_event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/window.h"
#include "evita/gc/local.h"
#include "evita/v8_glue/converter.h"

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
                                      const char* method_name) {
  auto const value = js_class->Get(gin::StringToV8(isolate, method_name));
  if (value.IsEmpty() || !value->IsFunction()) {
    LOG(0) << "Object " << V8ToString(js_class) << " has no method '" <<
        method_name << "', it has " <<
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
void EventHandler::DoDefaultEventHandling(EventTarget* event_target,
                                          Event* event) {
  const char kHandleEvent[] = "handleEvent";

  auto const isolate = controller_->isolate();
  v8::HandleScope handle_scope(isolate);

  auto const js_target = event_target->GetWrapper(isolate);
  auto const js_class = GetClassObject(isolate, js_target);
  if (js_class.IsEmpty())
    return;

  auto const js_method = ToMethodObject(isolate, js_class, kHandleEvent);
  if (js_method.IsEmpty())
    return;

  v8::TryCatch try_catch;
  v8::Handle<v8::Value> argv[1] { event->GetWrapper(isolate) };
  auto const value = js_method->CallAsFunction(js_target, 1, argv);
  if (value.IsEmpty())
    controller_->LogException(try_catch);
}

// ViewEventHandler
void EventHandler::DidDestroyWidget(WindowId window_id) {
  Window::DidDestroyWidget(window_id);
}

void EventHandler::DidDropWidget(WindowId source_id,
                                 WindowId target_id) {
  auto const source_window = FromWindowId(source_id);
  if (!source_window)
    return;
  auto const target_window = NewOrFromWindowId(target_id);
  if (!target_window)
    return;
  auto const event = new WindowEvent(L"dropwindow", Event::NotBubbling,
                                     Event::NotCancelable, source_window);
  CHECK(target_window->DispatchEvent(event));
  DoDefaultEventHandling(target_window, event);
}

void EventHandler::DidKillFocus(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  auto const event = new FocusEvent(L"blur", nullptr);
  window->DispatchEvent(event);
}

void EventHandler::DidRealizeWidget(WindowId window_id) {
  Window::DidRealizeWidget(window_id);
}

void EventHandler::DidSetFocus(WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  window->DidSetFocus();
  auto const event = new FocusEvent(L"focus", nullptr);
  window->DispatchEvent(event);
}

void EventHandler::DidStartHost() {
  controller_->DidStartHost();
}

void EventHandler::DispatchFormEvent(const ApiFormEvent& raw_event) {
  auto const target = EventTarget::FromEventTargetId(
      raw_event.target_id);
  if (!target)
    return;
  auto event = new FormEvent(raw_event.type, Event::Bubbling,
                             Event::NotCancelable,
                             raw_event.data);
  target->DispatchEvent(event);
}

void EventHandler::DispatchMouseEvent(const domapi::MouseEvent& api_event) {
  auto const window = EventTarget::FromEventTargetId(api_event.target_id);
  if (!window)
    return;
  gc::Local<MouseEvent> event(new MouseEvent(api_event));
  if (!window->DispatchEvent(event))
    return;
  DoDefaultEventHandling(window, event);
}

void EventHandler::OpenFile(WindowId window_id,
                            const base::string16& filename){
  controller_->OpenFile(window_id, filename);
}

void EventHandler::QueryClose(WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window)
    return;
  gc::Local<UiEvent> event(new UiEvent());
  event->InitUiEvent(L"queryclose", Event::NotBubbling, Event::Cancelable,
                     window, 0);
  if (!window->DispatchEvent(event))
    return;
  DoDefaultEventHandling(window, event);
}

void EventHandler::RunCallback(base::Closure callback) {
  callback.Run();
}

void EventHandler::WillDestroyHost() {
  controller_->WillDestroyHost();
}

} // namespace dom
