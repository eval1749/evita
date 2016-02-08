// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_WATCH_DOG_H_
#define EVITA_EDITOR_WATCH_DOG_H_

#include <atomic>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace base {
class PingProvider;
}

namespace editor {

//////////////////////////////////////////////////////////////////////
//
// WatchDog
//
class WatchDog final {
 public:
  class Watched;

  WatchDog();
  ~WatchDog();

  void Add(base::StringPiece16 name, base::PingProvider* provider);
  void Start();

 private:
  void DidFireTimer();
  void Ping();

  base::RepeatingTimer timer_;
  std::vector<std::unique_ptr<Watched>> watch_list_;

  DISALLOW_COPY_AND_ASSIGN(WatchDog);
};

}  // namespace editor

#endif  // EVITA_EDITOR_WATCH_DOG_H_
