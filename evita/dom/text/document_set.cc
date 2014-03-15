// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/document_set.h"

#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/strings/string_number_conversions.h"
#include "evita/dom/lock.h"
#include "evita/dom/text/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

namespace {

typedef std::pair<base::string16, base::string16> StringPair;

StringPair SplitByDot(const base::string16& name) {
  const auto last_dot = name.rfind('.');
  if (!last_dot || last_dot == base::string16::npos)
    return StringPair(name, L"");
  return StringPair(name.substr(0, last_dot), name.substr(last_dot));
}

} // namespace

//////////////////////////////////////////////////////////////////////
//
// DocumentSet::Observer
//
class DocumentSet::Observer {
  private: v8_glue::ScopedPersistent<v8::Function> function_;

  public: Observer(v8::Isolate* isolate, v8::Handle<v8::Function> function);
  public: ~Observer();

  public: v8::Local<v8::Function> GetCallback(v8::Isolate* isolate);
  public: void Notify(v8_glue::Runner* runner, const base::string16& type,
                      Document* document);

  DISALLOW_COPY_AND_ASSIGN(Observer);
};

DocumentSet::Observer::Observer(v8::Isolate* isolate,
                                v8::Handle<v8::Function> function)
    : function_(isolate, function) {
}

DocumentSet::Observer::~Observer() {
}

v8::Local<v8::Function> DocumentSet::Observer::GetCallback(
    v8::Isolate* isolate) {
  return function_.NewLocal(isolate);
}

void DocumentSet::Observer::Notify(v8_glue::Runner* runner,
                                   const base::string16& type,
                                   Document* document) {
  auto const isolate = runner->isolate();
  runner->Call(function_.NewLocal(runner->isolate()),
               v8::Undefined(isolate),
               gin::ConvertToV8(isolate, type),
               gin::ConvertToV8(isolate, document));
}

//////////////////////////////////////////////////////////////////////
//
// DocumentSet
//
DocumentSet::DocumentSet() {
}

DocumentSet::~DocumentSet() {
}

std::vector<Document*> DocumentSet::list() const {
  std::vector<Document*> list(map_.size());
  list.resize(0);
  for (const auto& pair : map_) {
    list.push_back(pair.second);
  }
  return std::move(list);
}

void DocumentSet::AddObserver(v8::Handle<v8::Function> callback) {
  auto const isolate = v8::Isolate::GetCurrent();
  for (auto const observer : observers_) {
    if (observer->GetCallback(isolate) == callback)
      return;
  }
  observers_.push_back(new Observer(isolate, callback));
}

Document* DocumentSet::Find(const base::string16 name) const {
  auto it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

base::string16 DocumentSet::MakeUniqueName(const base::string16& name) {
  if (!Find(name))
    return name;
  const auto pair = SplitByDot(name);
  auto candidate = name;
  for (auto n = 2; Find(candidate); ++ n) {
    candidate = pair.first + L" (" + base::IntToString16(n) + L")" +
      pair.second;
  }
  return candidate;
}

void DocumentSet::NotifyObserverWithInLock(const base::string16& type,
                                           Document* document) {
  DOM_AUTO_LOCK_SCOPE();
  std::vector<Observer*> observers(observers_);
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  for (auto const observer : observers) {
    observer->Notify(runner, type, document);
  }
}

void DocumentSet::Register(Document* document) {
  CHECK(!Find(document->name()));
  map_[document->name()] = document;
  ScheduleNotifyObserver(L"add", document);
}

void DocumentSet::RemoveObserver(v8::Handle<v8::Function> callback) {
  auto const isolate = v8::Isolate::GetCurrent();
  for (auto it = observers_.begin(); it != observers_.end(); ++it) {
    auto const observer = *it;
    if (observer->GetCallback(isolate) == callback) {
      observers_.erase(it);
      delete observer;
      return;
    }
  }
  observers_.push_back(new Observer(isolate, callback));
}

void DocumentSet::ResetForTesting() {
  map_.clear();
}

void DocumentSet::RenameDocument(Document* document,
                                 const base::string16& new_name) {
  auto& old_name = document->name();
  if (old_name == new_name)
   return;
  auto const it = map_.find(old_name);
  if (it == map_.end()) {
    document->buffer()->SetName(new_name);
    return;
  }
  const auto new_unique_name = MakeUniqueName(new_name);
  map_.erase(it);
  document->buffer()->SetName(new_unique_name);
  map_[new_unique_name] = document;
}

void DocumentSet::ScheduleNotifyObserver(const base::string16& type,
                                         Document* document) {
  auto const message_loop = base::MessageLoop::current();
  if (!message_loop) {
    // All unit tests except for explicitly construct |base::MessageLoop|
    // don't have message loop.
    return;
  }
  message_loop->PostTask(FROM_HERE, base::Bind(
      &DocumentSet::NotifyObserverWithInLock, base::Unretained(this), type,
      base::Unretained(document)));
}

void DocumentSet::Unregister(Document* document) {
  auto it = map_.find(document->name());
  if (it == map_.end()) {
    // We called |Document.remove()| for |document|.
    return;
  }
  map_.erase(it);
  ScheduleNotifyObserver(L"remove", document);
}

}  // namespace dom
