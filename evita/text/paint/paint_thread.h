// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_PAINT_PAINT_THREAD_H_
#define EVITA_TEXT_PAINT_PAINT_THREAD_H_

#include <memory>

#include "base/callback_forward.h"
#include "base/macros.h"

namespace base {
class Thread;
}

namespace gfx {
class DxDevice;
}

namespace base {
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
  void PostTask(const base::Location& from_here, base::OnceClosure task);
  void SchedulePaintTask(base::OnceClosure task);
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

#endif  // EVITA_TEXT_PAINT_PAINT_THREAD_H_
