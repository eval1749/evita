// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_handler.h"

#include "evita/dom/events/ui_event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/window.h"
#include "evita/gc/local.h"

namespace dom {

EventHandler::EventHandler(ScriptController* controller)
    : controller_(controller) {
}

EventHandler::~EventHandler() {
}

void EventHandler::DoDefaultEventHandling(EventTarget* event_target,
                                          Event* event) {
  auto const class_name = event_target->scriptable_class_name();
  const char method_name[] = "handleEvent";

  auto const isolate = controller_->isolate();
  v8::HandleScope handle_scope(isolate);

  auto const js_target = event_target->GetWrapper(isolate);
  auto const js_class = js_target->GetConstructor();
  if (js_class.IsEmpty()) {
    LOG(0) << "No such class " << class_name;
    return;
  }

  auto const js_method = js_class->ToObject()->Get(
      gin::StringToV8(isolate, method_name));
  if (js_method .IsEmpty()) {
    LOG(0) << "No such method " << class_name << "." << method_name;
    return;
  }

  v8::TryCatch try_catch;
  v8::Handle<v8::Value> argv[1] { event->GetWrapper(isolate) };
  auto const value = js_method->ToObject()->CallAsFunction(
      js_target, 1, argv);
  if (value.IsEmpty())
    controller_->LogException(try_catch);
}

// ViewEventHandler
void EventHandler::DidDestroyWidget(WindowId window_id) {
  Window::DidDestroyWidget(window_id);
}

void EventHandler::DidKillFocus(WindowId window_id) {
  Window::DidKillFocus(window_id);
}

void EventHandler::DidRealizeWidget(WindowId window_id) {
  Window::DidRealizeWidget(window_id);
}

void EventHandler::DidSetFocus(WindowId window_id) {
  Window::DidSetFocus(window_id);
}

void EventHandler::DidStartHost() {
  controller_->DidStartHost();
}

void EventHandler::OpenFile(WindowId window_id,
                            const base::string16& filename){
  controller_->OpenFile(window_id, filename);
}

void EventHandler::QueryClose(WindowId window_id) {
  auto const window = Window::FromWindowId(window_id);
  if (!window) {
    DVLOG(0) << "No such window " << window_id;
    return;
  }
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
