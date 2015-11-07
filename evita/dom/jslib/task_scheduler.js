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
      /** @const @type {!function()} */
      this.idleCallback_ = this.runTask.bind(this);

      /** @type {boolean} */
      this.isWaiting_ = false;

      /** @const @type {!Set.<Runnable>} */
      this.tasks_ = new Set();
    }

    /** @private */
    runTask() {
      this.isWaiting_ = false;
      if (this.tasks_.size === 0)
        return;
      const task = this.tasks_.values().next().value;
      if (!task)
        return;
      this.remove(task);
      task.run();
      this.wait();
    }

    /** @param {Runnable} task */
    remove(task) {
      this.tasks_.delete(task);
    }

    /** @param {Runnable} task */
    schedule(task) {
      this.tasks_.add(task);
      this.wait();
    }

    /** @private */
    wait() {
      if (this.isWaiting_ || this.tasks_.size === 0)
        return;
      this.isWaiting_ = true;
      Editor.requestIdleCallback(this.idleCallback_);
    }
  }

  global.TaskScheduler = TaskScheduler;
  global.taskScheduler = new TaskScheduler();
})();
