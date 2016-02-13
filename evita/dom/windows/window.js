// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('components.commander');

(function() {
  /** @type {string} */
  Window.prototype.status_;

  /** @type {boolean} */
  Window.prototype.visible_;

  /**
   * @param {!KeyboardEvent} event
   */
  function handleKeyDown(event) {
    const target = /** @type {!Window} */ (event.target);
    components.commander.Commander.instance.execute(target, event.keyCode);
  }

  /**
   * Default event handler.
   * @this {!Window}
   * @param {!Event} event
   */
  function handleEvent(event) {
    switch (event.type) {
      case Event.Names.HIDE:
        this.status_ = '';
        this.visible_ = false;
        return;
      case Event.Names.KEYDOWN:
        return handleKeyDown(/** @type {!KeyboardEvent} */ (event));
      case Event.Names.SHOW:
        this.visible_ = true;
        return;
    }
  }

  /**
   * @this {!Window}
   * @return {string}
   */
  function statusGet() { return this.status_; }

  /**
   * @this {!Window}
   */
  function statusSet(newStatus) {
    if (this.status_ === newStatus)
      return;
    this.status_ = newStatus;
    Editor.messageBox(this, this.status_, MessageBox.ICONINFORMATION);
  }

  Object.defineProperties(Window, {handleEvent: {value: handleEvent}});

  Object.defineProperties(Window.prototype, {
    focusTick_: {value: 0, writable: true},
    status: {get: statusGet, set: statusSet},
    status_: {value: '', writable: true},
    visible: {get: function() { return this.visible_; }},
    visible_: {value: false, writable: true}
  });
})();
