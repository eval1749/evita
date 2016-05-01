// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
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
    this.wait(0);
  }

  /** @param {Runnable} task */
  remove(task) { this.tasks_.delete(task); }

  /**
   * @param {Runnable} task
   * @param {number=} delay
   */
  schedule(task, delay = 0) {
    this.tasks_.add(task);
    this.wait(delay);
  }

  /**
   * @private
   * @param {number} delay
   */
  wait(delay) {
    if (this.isWaiting_ || this.tasks_.size === 0)
      return;
    this.isWaiting_ = true;
    Editor.requestIdleCallback(this.idleCallback_, {timeout: delay});
  }
}

/** @constructor */
core.TaskScheduler = TaskScheduler;
/** @const @type {!TaskScheduler} */
core.taskScheduler = new TaskScheduler();
});

/** @const @type {!core.TaskScheduler} */
var taskScheduler = core.taskScheduler;
