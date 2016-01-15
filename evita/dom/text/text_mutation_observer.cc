// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/dom/text/text_mutation_observer.h"

#include "evita/bindings/v8_glue_TextMutationObserverInit.h"
#include "evita/dom/lock.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_observer_controller.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/dom/script_host.h"
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
  TextDocument* const document_;
  text::Offset minimum_change_offset_;

  DISALLOW_COPY_AND_ASSIGN(Tracker);
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
  const auto minimum_change_offset = minimum_change_offset_;
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
  const auto tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void TextMutationObserver::DidInsertBefore(TextDocument* document,
                                           text::Offset offset,
                                           text::OffsetDelta length) {
  const auto tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->Update(offset);
}

void TextMutationObserver::DidMutateTextDocument(TextDocument* document) {
  const auto tracker = GetTracker(document);
  if (!tracker)
    return;
  if (!tracker->has_records())
    return;
  const auto runner = ScriptHost::instance()->runner();
  v8_glue::Runner::Scope runner_scope(runner);
  const auto isolate = runner->isolate();
  v8::Local<v8::Value> records;
  if (!gin::TryConvertToV8(isolate, tracker->TakeRecords(), &records))
    return;
  ASSERT_DOM_LOCKED();
  v8::TryCatch try_catch;
  try_catch.SetVerbose(true);
  runner->Call(callback_.NewLocal(isolate), v8::Undefined(isolate), records,
               gin::ConvertToV8(isolate, this));
  if (!try_catch.HasCaught())
    return;
  Disconnect();
  try_catch.ReThrow();
}

void TextMutationObserver::Disconnect() {
  TextMutationObserverController::instance()->Unregister(this);
}

TextMutationObserver::Tracker* TextMutationObserver::GetTracker(
    TextDocument* document) const {
  const auto& it = tracker_map_.find(document);
  return it == tracker_map_.end() ? nullptr : it->second.get();
}

void TextMutationObserver::Observe(TextDocument* document,
                                   const TextMutationObserverInit& options) {
  DCHECK(options.summary());
  if (!GetTracker(document)) {
    const auto& result = tracker_map_.emplace(
        document, std::move(std::make_unique<Tracker>(document)));
    DCHECK(result.second) << *document << " should be unique in tracker_map_";
  }
  TextMutationObserverController::instance()->Register(this, document);
}

std::vector<TextMutationRecord*> TextMutationObserver::TakeRecords() {
  std::vector<TextMutationRecord*> records;
  records.reserve(tracker_map_.size());
  for (const auto& key_val : tracker_map_) {
    const auto& tracker = key_val.second;
    for (const auto& record : tracker->TakeRecords())
      records.push_back(record);
  }
  return records;
}

}  // namespace dom
