// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/watch_dog.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "evita/base/ping_provider.h"
#include "evita/frames/frame.h"
#include "evita/frames/frame_list.h"

namespace editor {

using Watched = WatchDog::Watched;

//////////////////////////////////////////////////////////////////////
//
// WatchDog::Watched
//
class WatchDog::Watched final {
 public:
  Watched(base::StringPiece16 name, base::PingProvider* provider);
  ~Watched() = default;

  bool is_waiting() const { return is_waiting_.load(); }
  const base::string16& name() const { return name_; }
  const base::Time& last_ping_time() const { return last_ping_time_; }

  void Ping();

 private:
  std::atomic<bool> is_waiting_;
  base::Time last_ping_time_;
  const base::string16 name_;
  base::PingProvider* const provider_;

  DISALLOW_COPY_AND_ASSIGN(Watched);
};

Watched::Watched(base::StringPiece16 name, base::PingProvider* provider)
    : is_waiting_(false), name_(name.as_string()), provider_(provider) {}

void Watched::Ping() {
  DCHECK(!is_waiting());
  is_waiting_.store(true);
  last_ping_time_ = base::Time::Now();
  provider_->Ping(&is_waiting_);
}

//////////////////////////////////////////////////////////////////////
//
// WatchDog
//
WatchDog::WatchDog() {}
WatchDog::~WatchDog() {}

void WatchDog::Add(base::StringPiece16 name, base::PingProvider* provider) {
  watch_list_.emplace_back(new Watched(name, provider));
  watch_list_.back()->Ping();
}

void WatchDog::DidFireTimer() {
  const auto& now = base::Time::Now();
  for (const auto& watched : watch_list_) {
    if (!watched->is_waiting()) {
      watched->Ping();
      continue;
    }
    const auto& delta = now - watched->last_ping_time();
    if (delta < base::TimeDelta::FromSeconds(1))
      continue;
    const auto active_frame = views::FrameList::instance()->active_frame();
    if (!active_frame)
      continue;
    const auto& message =
        base::StringPrintf(L"%ls runs %ds. Ctrl+Break to terminate script.",
                           watched->name().c_str(), delta.InSeconds());
    active_frame->ShowMessage(MessageLevel_Warning, message);
  }
}

void WatchDog::Start() {
  timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1),
               base::Bind(&WatchDog::DidFireTimer, base::Unretained(this)));
}

}  // namespace editor
