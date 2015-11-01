// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @type {number} */
  const kIntervalMs = 100;

  //////////////////////////////////////////////////////////////////////
  //
  // TaskScheduler
  //
  class TaskScheduler {
    constructor() {
      /** @const @type {!Set.<Runnable>} */
      this.tasks_ = new Set();

      /** @const @type {!OneShotTimer} */
      this.timer_ = new OneShotTimer();
    }

    didFireTimer() {
      if (this.tasks_.size === 0)
        return;
      const task = this.tasks_.values().next().value;
      if (!task)
        return;
      this.remove(task);
      task.run();
      this.startTimerIfNeeded();
    }

    /** @param {Runnable} task */
    remove(task) {
      this.tasks_.delete(task);
    }

    /** @param {Runnable} task */
    schedule(task) {
      this.tasks_.add(task);
      this.startTimerIfNeeded();
    }

    /** @private */
    startTimerIfNeeded() {
      if (this.tasks_.size === 0 || this.timer_.isRunning)
        return;
      this.timer_.start(kIntervalMs, this.didFireTimer, this);
    }
  }

  global.TaskScheduler = TaskScheduler;
  global.taskScheduler = new TaskScheduler();
})();
