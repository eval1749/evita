// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "evita/dom/text/text_mutation_observer.h"

#include "base/memory/weak_ptr.h"
#include "evita/dom/bindings/ginx_TextMutationObserverInit.h"
#include "evita/dom/lock.h"
#include "evita/dom/scheduler/micro_task.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/ginx/runner.h"
#include "evita/text/buffer.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/static_range.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver::Tracker holds text document mutations.
//
class TextMutationObserver::Tracker final
    : public text::BufferMutationObserver {
 public:
  Tracker(TextMutationObserver* observer, TextDocument* document);
  ~Tracker() final;

  bool has_records() const { return number_of_mutations_ > 0; }

  std::vector<TextMutationRecord*> TakeRecords();

 private:
  base::WeakPtr<Tracker> GetWeakPtr();
  static void NotifyMutations(base::WeakPtr<Tracker> tracker);
  // Resets summary tracking.
  void ResetSummary();
  void RunCallback();
  void ScheduleNotify();

  // text::BufferMutationObserver
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  // TODO(eval1749): Reference to |TextDocument| from |Tracker| should be a weak
  // reference.
  TextDocument* const document_;

  // A document end offset at start of recording.
  text::OffsetDelta delta_;

  // A number of unmodified characters from start of document.
  text::OffsetDelta head_count_;

  bool is_scheduled_ = false;

  int number_of_mutations_ = 0;

  TextMutationObserver* const observer_;

  // A number of unmodified characters from end of document.
  text::OffsetDelta tail_count_;

  base::WeakPtrFactory<Tracker> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Tracker);
};

TextMutationObserver::Tracker::Tracker(TextMutationObserver* observer,
                                       TextDocument* document)
    : document_(document), observer_(observer), weak_factory_(this) {
  document_->buffer()->AddObserver(this);
  ResetSummary();
}

TextMutationObserver::Tracker::~Tracker() {
  document_->buffer()->RemoveObserver(this);
}

void TextMutationObserver::Tracker::DidDeleteAt(
    const text::StaticRange& range) {
  ScheduleNotify();
  ++number_of_mutations_;
  const auto offset = text::OffsetDelta(range.start().value());
  delta_ = delta_ - range.length();
  head_count_ = std::min(head_count_, offset);
  tail_count_ =
      std::min(tail_count_, text::OffsetDelta(document_->length()) - offset);
}

void TextMutationObserver::Tracker::DidInsertBefore(
    const text::StaticRange& range) {
  ScheduleNotify();
  ++number_of_mutations_;
  delta_ = delta_ + range.length();
  const auto offset = text::OffsetDelta(range.start().value());
  head_count_ = std::min(head_count_, offset);
  tail_count_ = std::min(tail_count_, text::OffsetDelta(document_->length() -
                                                        range.end().value()));
}

base::WeakPtr<TextMutationObserver::Tracker>
TextMutationObserver::Tracker::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void TextMutationObserver::Tracker::NotifyMutations(
    base::WeakPtr<Tracker> tracker) {
  ASSERT_DOM_LOCKED();
  if (!tracker)
    return;
  tracker->RunCallback();
}

void TextMutationObserver::Tracker::ResetSummary() {
  number_of_mutations_ = 0;
  delta_ = text::OffsetDelta();
  tail_count_ = text::OffsetDelta(document_->length());
  head_count_ = text::OffsetDelta(document_->length());
}

void TextMutationObserver::Tracker::RunCallback() {
  is_scheduled_ = false;
  if (!has_records())
    return;
  const auto runner = ScriptHost::instance()->runner();
  ginx::Runner::Scope runner_scope(runner);
  const auto isolate = runner->isolate();
  v8::Local<v8::Value> records;
  if (!gin::TryConvertToV8(isolate, TakeRecords(), &records)) {
    LOG(FATAL) << "Failed to convert mutation records";
    return;
  }
  runner->CallAsFunction(observer_->callback_.NewLocal(isolate),
                         v8::Undefined(isolate), records,
                         gin::ConvertToV8(isolate, observer_));
}

void TextMutationObserver::Tracker::ScheduleNotify() {
  if (is_scheduled_)
    return;
  is_scheduled_ = true;
  ScriptHost::instance()->EnqueueMicroTask(std::make_unique<MicroTask>(
      FROM_HERE, base::Bind(&Tracker::NotifyMutations, GetWeakPtr())));
}

std::vector<TextMutationRecord*> TextMutationObserver::Tracker::TakeRecords() {
  if (!has_records())
    return std::vector<TextMutationRecord*>();
  const auto record = new TextMutationRecord(L"summary", document_, delta_,
                                             head_count_, tail_count_);
  ResetSummary();
  return std::vector<TextMutationRecord*>{record};
}

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserver
//
TextMutationObserver::TextMutationObserver(v8::Local<v8::Function> callback)
    : callback_(v8::Isolate::GetCurrent(), callback) {}

TextMutationObserver::~TextMutationObserver() {}

void TextMutationObserver::Disconnect() {
  trackers_.clear();
}

void TextMutationObserver::Observe(TextDocument* document,
                                   const TextMutationObserverInit& options) {
  DCHECK(options.summary());
  trackers_.emplace_back(std::move(std::make_unique<Tracker>(this, document)));
}

std::vector<TextMutationRecord*> TextMutationObserver::TakeRecords() {
  std::vector<TextMutationRecord*> records;
  records.reserve(trackers_.size());
  for (const auto& tracker : trackers_) {
    for (const auto& record : tracker->TakeRecords())
      records.emplace_back(record);
  }
  return records;
}

}  // namespace dom
