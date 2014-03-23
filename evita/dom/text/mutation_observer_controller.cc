// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/mutation_observer_controller.h"

#include <unordered_set>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/buffer.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/mutation_observer.h"
#include "evita/dom/text/mutation_record.h"
#include "evita/text/buffer.h"
#include "evita/text/buffer_mutation_observer.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// MutationObserverController::Tracker
// Instances of |Tracker| track document mutation of assocaited |Document|.
//
class MutationObserverController::Tracker final
    : public base::RefCounted<Tracker>,
      public text::BufferMutationObserver {

  private: gc::Member<Document> document_;
  private: bool is_observing_;
  private: bool is_schedule_notification_;
  private: std::unordered_set<MutationObserver*> observers_;
  private: base::WeakPtrFactory<Tracker> weak_factory_;

  public: Tracker(Document* document);
  public: virtual ~Tracker();

  public: bool is_tracking() const { return !observers_.empty(); }

  private: base::WeakPtr<Tracker> GetWeakPtr();
  public: void Register(MutationObserver* observer);
  private: static void NotifyObservers(base::WeakPtr<Tracker> tracker);
  private: void ScheduleNotification();
  public: void Unregister(MutationObserver* observer);

  // text::BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(Tracker);
};

MutationObserverController::Tracker::Tracker(Document* document)
    : document_(document), is_observing_(false),
      is_schedule_notification_(false),
      weak_factory_(this) {
}

MutationObserverController::Tracker::~Tracker() {
  if (is_observing_)
    document_->buffer()->RemoveObserver(this);
}

base::WeakPtr<MutationObserverController::Tracker>
    MutationObserverController::Tracker::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void MutationObserverController::Tracker::NotifyObservers(
    base::WeakPtr<Tracker> tracker) {
  if (!tracker)
    return;
  tracker->is_schedule_notification_ = false;
  std::vector<MutationObserver*> observers;
  for (auto observer : tracker->observers_) {
    observers.push_back(observer);
  }
  scoped_refptr<Tracker> protect(tracker.get());
  DOM_AUTO_LOCK_SCOPE();
  for (auto observer : observers) {
    observer->DidMutateDocument(tracker->document_);
  }
}

void MutationObserverController::Tracker::Register(
    MutationObserver* observer) {
  observers_.insert(observer);
  if (is_observing_)
    return;
  document_->buffer()->AddObserver(this);
  is_observing_ = true;
}

void MutationObserverController::Tracker::ScheduleNotification() {
  if (is_schedule_notification_)
    return;
  is_schedule_notification_ = true;
  ScriptHost::instance()->PostTask(FROM_HERE,
      base::Bind(&Tracker::NotifyObservers, GetWeakPtr()));
}

void MutationObserverController::Tracker::Unregister(
    MutationObserver* observer) {
  observers_.erase(observer);
  if (!observers_.empty())
    return;
  document_->buffer()->RemoveObserver(this);
  is_observing_ = false;
}

// text::BufferMutationObserver
void MutationObserverController::Tracker::DidDeleteAt(Posn offset,
                                                      size_t length) {
  for (auto observer : observers_) {
    observer->DidDeleteAt(document_, offset, length);
  }
  ScheduleNotification();
}

void MutationObserverController::Tracker::DidInsertAt(Posn offset,
                                                      size_t length) {
  for (auto observer : observers_) {
    observer->DidInsertAt(document_, offset, length);
  }
  ScheduleNotification();
}

//////////////////////////////////////////////////////////////////////
//
// MutationObserverController
//
MutationObserverController::MutationObserverController() {
}

MutationObserverController::~MutationObserverController() {
}

void MutationObserverController::Register(MutationObserver* observer,
                                          Document* document) {
  auto const it = map_.find(document);
  if (it != map_.end()) {
    it->second->Register(observer);
    return;
  }
  auto const tracker = new Tracker(document);
  map_[document] = tracker;
  tracker->AddRef();
  tracker->Register(observer);
}

void MutationObserverController::Unregister(MutationObserver* observer) {
  std::vector<Document*> keys_to_remove;
  for (auto key_val : map_) {
    auto const tracker = key_val.second;
    tracker->Unregister(observer);
    if (!tracker->is_tracking())
      keys_to_remove.push_back(key_val.first);
  }
  for (auto document : keys_to_remove) {
    auto const it = map_.find(document);
    if (it == map_.end())
      continue;
    it->second->Release();
    map_.erase(document);
  }
}

}  // namespace dom
