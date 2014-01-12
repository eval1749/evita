// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_handler.h"

#include "evita/dom/script_controller.h"
#include "evita/dom/window.h"

namespace dom {

EventHandler::EventHandler(ScriptController* controller)
    : controller_(controller) {
}

EventHandler::~EventHandler() {
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

void EventHandler::RunCallback(base::Closure callback) {
  callback.Run();
}

void EventHandler::WillDestroyHost() {
  controller_->WillDestroyHost();
}

} // namespace dom
