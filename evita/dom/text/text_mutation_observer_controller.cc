// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_mutation_observer_controller.h"

#include <unordered_set>
#include <vector>

#pragma warning(push)
#pragma warning(disable : 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_observer.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/text/buffer.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/static_range.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserverController::Tracker
// Instances of |Tracker| track document mutation of associated |TextDocument|.
//
class TextMutationObserverController::Tracker final
    : public base::RefCounted<Tracker>,
      public text::BufferMutationObserver {
 public:
  explicit Tracker(TextDocument* document);
  ~Tracker() final;

  bool is_tracking() const { return !observers_.empty(); }

  void Register(TextMutationObserver* observer);
  void Unregister(TextMutationObserver* observer);

 private:
  base::WeakPtr<Tracker> GetWeakPtr();
  static void NotifyObservers(base::WeakPtr<Tracker> tracker);
  void ScheduleNotification();

  // text::BufferMutationObserver
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  gc::Member<TextDocument> document_;
  bool is_observing_;
  bool is_schedule_notification_;
  std::unordered_set<TextMutationObserver*> observers_;
  base::WeakPtrFactory<Tracker> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Tracker);
};

TextMutationObserverController::Tracker::Tracker(TextDocument* document)
    : document_(document),
      is_observing_(false),
      is_schedule_notification_(false),
      weak_factory_(this) {}

TextMutationObserverController::Tracker::~Tracker() {
  if (is_observing_)
    document_->buffer()->RemoveObserver(this);
}

base::WeakPtr<TextMutationObserverController::Tracker>
TextMutationObserverController::Tracker::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void TextMutationObserverController::Tracker::NotifyObservers(
    base::WeakPtr<Tracker> tracker) {
  if (!tracker)
    return;
  tracker->is_schedule_notification_ = false;
  std::vector<TextMutationObserver*> observers;
  for (const auto& observer : tracker->observers_) {
    observers.push_back(observer);
  }
  scoped_refptr<Tracker> protect(tracker.get());
  DOM_AUTO_LOCK_SCOPE();
  for (const auto& observer : observers) {
    observer->DidMutateTextDocument(tracker->document_);
  }
}

void TextMutationObserverController::Tracker::Register(
    TextMutationObserver* observer) {
  observers_.insert(observer);
  if (is_observing_)
    return;
  document_->buffer()->AddObserver(this);
  is_observing_ = true;
}

void TextMutationObserverController::Tracker::ScheduleNotification() {
  if (is_schedule_notification_)
    return;
  is_schedule_notification_ = true;
  ScriptHost::instance()->ScheduleIdleTask(
      base::Bind(&Tracker::NotifyObservers, GetWeakPtr()));
}

void TextMutationObserverController::Tracker::Unregister(
    TextMutationObserver* observer) {
  observers_.erase(observer);
  if (!observers_.empty())
    return;
  document_->buffer()->RemoveObserver(this);
  is_observing_ = false;
}

// text::BufferMutationObserver
void TextMutationObserverController::Tracker::DidDeleteAt(
    const text::StaticRange& range) {
  for (const auto& observer : observers_)
    observer->DidDeleteAt(document_, range.start(), range.length());
  ScheduleNotification();
}

void TextMutationObserverController::Tracker::DidInsertBefore(
    const text::StaticRange& range) {
  for (const auto& observer : observers_)
    observer->DidInsertBefore(document_, range.start(), range.length());
  ScheduleNotification();
}

//////////////////////////////////////////////////////////////////////
//
// TextMutationObserverController
//
TextMutationObserverController::TextMutationObserverController() {}

TextMutationObserverController::~TextMutationObserverController() {}

void TextMutationObserverController::Register(TextMutationObserver* observer,
                                              TextDocument* document) {
  const auto it = map_.find(document);
  if (it != map_.end()) {
    it->second->Register(observer);
    return;
  }
  const auto tracker = new Tracker(document);
  const auto& result = map_.emplace(document, tracker);
  DCHECK(result.second) << "Tracker should be a singleton: " << document;
  tracker->AddRef();
  tracker->Register(observer);
}

void TextMutationObserverController::Unregister(
    TextMutationObserver* observer) {
  std::vector<TextDocument*> keys_to_remove;
  for (const auto& key_val : map_) {
    const auto tracker = key_val.second;
    tracker->Unregister(observer);
    if (!tracker->is_tracking())
      keys_to_remove.push_back(key_val.first);
  }
  for (const auto& document : keys_to_remove) {
    const auto it = map_.find(document);
    if (it == map_.end())
      continue;
    it->second->Release();
    map_.erase(document);
  }
}

}  // namespace dom
