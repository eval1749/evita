// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/mutation_observer.h"

#include <vector>

#include "evita/bindings/MutationObserverInit.h"
#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/mutation_observer_controller.h"
#include "evita/dom/text/mutation_record.h"
#include "evita/dom/script_host.h"
#include "evita/ed_defs.h"
#include "evita/v8_glue/runner.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// MutationObserverClass
//
class MutationObserverClass : public v8_glue::WrapperInfo {
  public: MutationObserverClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~MutationObserverClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &MutationObserverClass::NewMutationObserver);
  }

  private: static MutationObserver* NewMutationObserver(
      v8::Handle<v8::Function> callback) {
    return new MutationObserver(callback);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("disconnet", &MutationObserver::Disconnect)
        .SetMethod("observe", &MutationObserver::Observe)
        .SetMethod("takeRecords", &MutationObserver::TakeRecords);
  }

    DISALLOW_COPY_AND_ASSIGN(MutationObserverClass);
};
}   // namespace

//////////////////////////////////////////////////////////////////////
//
// MutationObserver::Tracker
//
class MutationObserver::Tracker {
  // TODO(yosi) Reference to |Document| from |Tracker| should be a weak
  // reference.
  private: Document* document_;
  private: text::Posn minimum_change_offset_;

  public: Tracker(Document* document);
  public: ~Tracker();

  public: bool has_records() const {
    return minimum_change_offset_ != text::Posn_Max;
  }

  public: void Reset();
  public: std::vector<MutationRecord*> TakeRecords();
  public: void Update(text::Posn offset);
};

MutationObserver::Tracker::Tracker(Document* document) : document_(document) {
  Reset();
}

MutationObserver::Tracker::~Tracker() {
}

void MutationObserver::Tracker::Reset() {
  minimum_change_offset_ = text::Posn_Max;
}

std::vector<MutationRecord*> MutationObserver::Tracker::TakeRecords() {
  if (!has_records())
    return std::vector<MutationRecord*>();
  auto const minimum_change_offset = minimum_change_offset_;
  Reset();
  return std::vector<MutationRecord*> {
    new MutationRecord(L"summary", document_, minimum_change_offset),
  };
}

void MutationObserver::Tracker::Update(text::Posn offset) {
  minimum_change_offset_ = std::min(minimum_change_offset_, offset);
}

//////////////////////////////////////////////////////////////////////
//
// MutationObserver
//
DEFINE_SCRIPTABLE_OBJECT(MutationObserver, MutationObserverClass);

MutationObserver::MutationObserver(v8::Handle<v8::Function> callback)
    : callback_(v8::Isolate::GetCurrent(), callback) {
}

MutationObserver::~MutationObserver() {
}

void MutationObserver::DidDeleteAt(Document* document, Posn offset, size_t) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void MutationObserver::DidInsertAt(Document* document, Posn offset, size_t) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void MutationObserver::DidMutateDocument(Document* document) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  if (!tracker->has_records())
    return;
  auto const runner = ScriptHost::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const isolate = runner->isolate();
  runner->Call(callback_.NewLocal(isolate), v8::Undefined(isolate),
               gin::ConvertToV8(isolate, tracker->TakeRecords()),
               gin::ConvertToV8(isolate, this));
}

void MutationObserver::Disconnect() {
  MutationObserverController::instance()->Unregister(this);
}

MutationObserver::Tracker* MutationObserver::GetTracker(
    Document* document) const {
  auto const it = tracker_map_.find(document);
  return it == tracker_map_.end() ? nullptr : it->second;
}

void MutationObserver::Observe(Document* document,
                               const MutationObserverInit& options) {
  DCHECK(options.summary());
  __assume(options.summary());
  tracker_map_[document] = new Tracker(document);
  MutationObserverController::instance()->Register(this, document);
}

std::vector<MutationRecord*> MutationObserver::TakeRecords() {
  std::vector<MutationRecord*> records;
  for (auto key_val : tracker_map_) {
   auto const tracker = key_val.second;
   for (auto record : tracker->TakeRecords()) {
     records.push_back(record);
   }
  }

  return records;
}

}  // namespace dom
