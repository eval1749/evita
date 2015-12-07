// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PAINT_THREAD_H_
#define EVITA_PAINT_PAINT_THREAD_H_

#include <memory>

#include "base/basictypes.h"
#include "base/callback_forward.h"

namespace base {
class Thread;
}

namespace gfx {
class DxDevice;
}

namespace tracked_objects {
class Location;
}

namespace ui {
class Compositor;
}

namespace paint {

class PaintThread final {
 public:
  PaintThread();
  ~PaintThread();

  ui::Compositor* compositor() const;
  gfx::DxDevice* device() const;
  static PaintThread* instance();

  void CommitIfNeeded();
  void PostTask(const tracked_objects::Location& from_here,
                const base::Closure& task);
  void SchedulePaintTask(const base::Closure& task);
  void Start();

 private:
  class Scheduler;

  void DidStartThread();

  std::unique_ptr<ui::Compositor> compositor_;
  std::unique_ptr<gfx::DxDevice> device_;
  const std::unique_ptr<Scheduler> scheduler_;
  const std::unique_ptr<base::Thread> thread_;

  DISALLOW_COPY_AND_ASSIGN(PaintThread);
};

}  // namespace paint

#endif  // EVITA_PAINT_PAINT_THREAD_H_
