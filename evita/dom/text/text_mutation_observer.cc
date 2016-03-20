// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/dom/text/text_mutation_observer.h"

#include "evita/dom/bindings/ginx_TextMutationObserverInit.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_observer_controller.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/ginx/runner.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver::Tracker holds text document mutations.
//
class TextMutationObserver::Tracker final {
 public:
  explicit Tracker(TextDocument* document);
  ~Tracker();

  bool has_records() const { return number_of_mutations_ > 0; }

  std::vector<TextMutationRecord*> TakeRecords();
  void DidDeleteAt(text::Offset offset, text::OffsetDelta length);
  void DidInsertBefore(text::Offset offset, text::OffsetDelta length);

 private:
  // Resets summary tracking.
  void ResetRecording();

  // TODO(eval1749): Reference to |TextDocument| from |Tracker| should be a weak
  // reference.
  TextDocument* const document_;

  // A document end offset at start of recording.
  text::OffsetDelta document_length_;

  // A number of mutations holds in this tracker.
  int number_of_mutations_ = 0;

  // A number of unmodified characters from start of document.
  text::OffsetDelta head_count_;

  // A number of unmodified characters from end of document.
  text::OffsetDelta tail_count_;

  DISALLOW_COPY_AND_ASSIGN(Tracker);
};

TextMutationObserver::Tracker::Tracker(TextDocument* document)
    : document_(document) {
  ResetRecording();
}

TextMutationObserver::Tracker::~Tracker() {}

void TextMutationObserver::Tracker::DidDeleteAt(text::Offset offset,
                                                text::OffsetDelta length) {
  ++number_of_mutations_;
  head_count_ = std::min(head_count_, text::OffsetDelta(offset.value()));
  tail_count_ = std::min(
      tail_count_, text::OffsetDelta(document_->length() - offset.value()));
}

void TextMutationObserver::Tracker::DidInsertBefore(text::Offset offset,
                                                    text::OffsetDelta length) {
  ++number_of_mutations_;
  head_count_ = std::min(head_count_, text::OffsetDelta(offset.value()));
  tail_count_ = std::min(
      tail_count_,
      text::OffsetDelta(document_->length() - offset.value() - length.value()));
}

void TextMutationObserver::Tracker::ResetRecording() {
  number_of_mutations_ = 0;
  document_length_ = text::OffsetDelta(document_->length());
  tail_count_ = document_length_;
  head_count_ = document_length_;
}

std::vector<TextMutationRecord*> TextMutationObserver::Tracker::TakeRecords() {
  if (!has_records())
    return std::vector<TextMutationRecord*>();
  const auto record = new TextMutationRecord(
      L"summary", document_, document_length_, head_count_, tail_count_);
  ResetRecording();
  return std::vector<TextMutationRecord*>{record};
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
  tracker->DidDeleteAt(offset, length);
}

void TextMutationObserver::DidInsertBefore(TextDocument* document,
                                           text::Offset offset,
                                           text::OffsetDelta length) {
  const auto tracker = GetTracker(document);
  if (!tracker)
    return;
  tracker->DidInsertBefore(offset, length);
}

void TextMutationObserver::DidMutateTextDocument(TextDocument* document) {
  const auto tracker = GetTracker(document);
  if (!tracker)
    return;
  if (!tracker->has_records())
    return;
  const auto runner = ScriptHost::instance()->runner();
  ginx::Runner::Scope runner_scope(runner);
  const auto isolate = runner->isolate();
  v8::Local<v8::Value> records;
  if (!gin::TryConvertToV8(isolate, tracker->TakeRecords(), &records))
    return;
  ASSERT_DOM_LOCKED();
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  runner->CallAsFunction(callback_.NewLocal(isolate), v8::Undefined(isolate),
                         records, gin::ConvertToV8(isolate, this));
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
