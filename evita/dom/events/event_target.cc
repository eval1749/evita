// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event_target.h"

#include <list>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "common/adopters/reverse.h"
#include "evita/dom/lock.h"
#include "evita/dom/events/event.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

namespace {

v8::Handle<v8::Object> GetCallee(v8::Isolate* isolate,
                                 v8::Handle<v8::Value> object) {
  if (!object->IsObject())
    return v8::Handle<v8::Object>();
  if (object->ToObject()->IsCallable())
    return object->ToObject();
  auto handler =
      object->ToObject()->Get(gin::StringToV8(isolate, "handleEvent"));
  if (handler.IsEmpty() || !handler->IsObject())
    return v8::Handle<v8::Object>();
  return handler->ToObject();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EventTarget::EventListenerMap
//
class EventTarget::EventListenerMap final {
 public:
  struct EventListener final {
    bool capture;
    v8_glue::ScopedPersistent<v8::Object> callback;

    EventListener(v8::Isolate* isoalte,
                  v8::Handle<v8::Object> callback,
                  bool capture)
        : capture(capture), callback(isoalte, callback) {}

   private:
    DISALLOW_COPY_AND_ASSIGN(EventListener);
  };

  using EventListenerList = std::list<EventListener*>;

  EventListenerMap() = default;
  ~EventListenerMap() = default;

  void Add(const base::string16& type,
           v8::Handle<v8::Object> callback,
           bool capture);
  EventListenerList* Find(const base::string16& type) const;
  void Remove(const base::string16& type,
              v8::Handle<v8::Object> callback,
              bool capture);

 private:
  std::unordered_map<base::string16, EventListenerList*> map_;

  DISALLOW_COPY_AND_ASSIGN(EventListenerMap);
};

void EventTarget::EventListenerMap::Add(const base::string16& type,
                                        v8::Handle<v8::Object> callback,
                                        bool capture) {
  auto isolate = v8::Isolate::GetCurrent();
  if (auto const list = Find(type)) {
    for (auto runner : *list) {
      if (runner->callback == callback && runner->capture == capture) {
        runner->callback.Reset(isolate, callback);
        return;
      }
    }
    list->push_back(new EventListener(isolate, callback, capture));
    return;
  }

  auto list = new EventListenerList();
  list->push_back(new EventListener(isolate, callback, capture));
  map_[type] = list;
}

EventTarget::EventListenerMap::EventListenerList*
EventTarget::EventListenerMap::Find(const base::string16& type) const {
  auto present = map_.find(type);
  return present == map_.end() ? nullptr : present->second;
}

void EventTarget::EventListenerMap::Remove(const base::string16& type,
                                           v8::Handle<v8::Object> callback,
                                           bool capture) {
  auto list = Find(type);
  if (!list)
    return;
  for (auto it = list->begin(); it != list->end(); ++it) {
    if ((*it)->callback == callback && (*it)->capture == capture) {
      list->erase(it);
      return;
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
EventTarget::EventTarget() : event_listener_map_(new EventListenerMap()) {}

EventTarget::~EventTarget() {}

void EventTarget::AddEventListener(const base::string16& type,
                                   v8::Handle<v8::Object> listener,
                                   bool capture) {
  event_listener_map_->Add(type, listener, capture);
}

void EventTarget::AddEventListener(const base::string16& type,
                                   v8::Handle<v8::Object> listener) {
  AddEventListener(type, listener, false);
}

EventTarget::EventPath EventTarget::BuildEventPath() const {
  return std::vector<EventTarget*>();
}

bool EventTarget::DispatchEvent(Event* event) {
  auto const runner = ScriptHost::instance()->runner();

  if (event->dispatched() || event->event_phase() != Event::kNone ||
      event->type().empty()) {
    ScriptHost::instance()->ThrowError("InvalidStateError");
    return false;
  }

  Event::DispatchScope dispatch_scope(event, this);

  auto event_path = BuildEventPath();
  for (auto target : event_path) {
    DCHECK(target != this);
    dispatch_scope.set_current_target(target);
    target->InvokeEventListeners(runner, event);
    if (event->stop_propagation())
      break;
  }

  dispatch_scope.StartAtTarget();
  dispatch_scope.set_current_target(this);
  InvokeEventListeners(runner, event);

  if (event->bubbles()) {
    dispatch_scope.StartBubbling();
    for (auto target : common::adopters::reverse(event_path)) {
      DCHECK(target != this);
      if (event->stop_propagation())
        break;
      dispatch_scope.set_current_target(target);
      target->InvokeEventListeners(runner, event);
    }
  }
  return !event->default_prevented();
}

void EventTarget::DispatchEventWithInLock(Event* event) {
  auto const runner = ScriptHost::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  v8::TryCatch try_catch;
  // We should prevent UI thread to access DOM.
  DOM_AUTO_LOCK_SCOPE();
  DispatchEvent(event);
  runner->HandleTryCatch(try_catch);
}

void EventTarget::InvokeEventListeners(v8_glue::Runner* runner, Event* event) {
  auto listeners = event_listener_map_->Find(event->type());
  if (!listeners)
    return;
  auto listeners_copy = *listeners;

  for (auto listener : listeners_copy) {
    if (event->stop_immediate_propagation())
      return;
    if (event->event_phase() == Event::kCapturingPhase) {
      if (!listener->capture)
        continue;
    } else if (event->event_phase() == Event::kBubblingPhase) {
      if (listener->capture)
        continue;
    }

    auto const isolate = runner->isolate();
    auto const callee =
        GetCallee(isolate, listener->callback.NewLocal(isolate));
    if (callee.IsEmpty())
      continue;
    runner->Call(callee, GetWrapper(isolate), event->GetWrapper(isolate));
  }
}

void EventTarget::RemoveEventListener(const base::string16& type,
                                      EventListener listener,
                                      bool capture) {
  event_listener_map_->Remove(type, listener, capture);
}

void EventTarget::RemoveEventListener(const base::string16& type,
                                      EventListener listener) {
  RemoveEventListener(type, listener, false);
}

// TODO(eval1749): We should get rid of
// |EventTarget::ScheduleDispatchEventDeprecated()|.
void EventTarget::ScheduleDispatchEventDeprecated(Event* event) {
  ScriptHost::instance()->PostTaskDeprecated(
      FROM_HERE, base::Bind(&EventTarget::DispatchEventWithInLock,
                            base::Unretained(this), base::Unretained(event)));
}

}  // namespace dom
