// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_mutation_observer.h"

#include <algorithm>
#include <vector>

#include "evita/bindings/v8_glue_TextMutationObserverInit.h"
#include "evita/dom/lock.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_observer_controller.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/dom/script_host.h"
#include "evita/ed_defs.h"
#include "evita/v8_glue/runner.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver::Tracker
//
class TextMutationObserver::Tracker final {
 public:
  explicit Tracker(TextDocument* document);
  ~Tracker();

  bool has_records() const {
    return minimum_change_offset_ != text::Offset::Max();
  }

  void Reset();
  std::vector<TextMutationRecord*> TakeRecords();
  void Update(text::Offset offset);

 private:
  // TODO(eval1749): Reference to |TextDocument| from |Tracker| should be a weak
  // reference.
  TextDocument* document_;
  text::Offset minimum_change_offset_;
};

TextMutationObserver::Tracker::Tracker(TextDocument* document)
    : document_(document) {
  Reset();
}

TextMutationObserver::Tracker::~Tracker() {}

void TextMutationObserver::Tracker::Reset() {
  minimum_change_offset_ = text::Offset::Max();
}

std::vector<TextMutationRecord*> TextMutationObserver::Tracker::TakeRecords() {
  if (!has_records())
    return std::vector<TextMutationRecord*>();
  auto const minimum_change_offset = minimum_change_offset_;
  Reset();
  return std::vector<TextMutationRecord*>{
      new TextMutationRecord(L"summary", document_, minimum_change_offset),
  };
}

void TextMutationObserver::Tracker::Update(text::Offset offset) {
  minimum_change_offset_ = std::min(minimum_change_offset_, offset);
}

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver
//
TextMutationObserver::TextMutationObserver(v8::Local<v8::Function> callback)
    : callback_(v8::Isolate::GetCurrent(), callback) {}

TextMutationObserver::~TextMutationObserver() {}

void TextMutationObserver::DidDeleteAt(TextDocument* document,
                                       text::Offset offset,
                                       text::OffsetDelta length) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void TextMutationObserver::DidInsertBefore(TextDocument* document,
                                           text::Offset offset,
                                           text::OffsetDelta length) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void TextMutationObserver::DidMutateTextDocument(TextDocument* document) {
  auto const tracker = GetTracker(document);
  if (!tracker)
    return;
  if (!tracker->has_records())
    return;
  auto const runner = ScriptHost::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const isolate = runner->isolate();
  v8::Local<v8::Value> records;
  if (!gin::TryConvertToV8(isolate, tracker->TakeRecords(), &records))
    return;
  ASSERT_DOM_LOCKED();
  runner->Call(callback_.NewLocal(isolate), v8::Undefined(isolate), records,
               gin::ConvertToV8(isolate, this));
}

void TextMutationObserver::Disconnect() {
  TextMutationObserverController::instance()->Unregister(this);
}

TextMutationObserver::Tracker* TextMutationObserver::GetTracker(
    TextDocument* document) const {
  auto const it = tracker_map_.find(document);
  return it == tracker_map_.end() ? nullptr : it->second;
}

void TextMutationObserver::Observe(TextDocument* document,
                                   const TextMutationObserverInit& options) {
  DCHECK(options.summary());
  __assume(options.summary());
  tracker_map_[document] = new Tracker(document);
  TextMutationObserverController::instance()->Register(this, document);
}

std::vector<TextMutationRecord*> TextMutationObserver::TakeRecords() {
  std::vector<TextMutationRecord*> records;
  for (auto key_val : tracker_map_) {
    auto const tracker = key_val.second;
    for (auto record : tracker->TakeRecords()) {
      records.push_back(record);
    }
  }

  return records;
}

}  // namespace dom
