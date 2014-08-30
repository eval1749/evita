// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  /** @type {string} */
  Window.prototype.status_;

  /**
   * Default event handler.
   * @this {!Window}
   * @param {!Event} event
   */
  function handleEvent(event) {
    commander.handleEvent(event);
  }

  /**
   * @this {!Window}
   * @return {string}
   */
  function statusGet() {
    return this.status;
  }

  /**
   * @this {!Window}
   */
  function statusSet(newStatus) {
    if (this.status_ == newStatus)
      return;
    this.status_ = newStatus;
    Editor.messageBox(this, this.status_, MessageBox.ICONINFORMATION);
  }

  Object.defineProperties(Window.prototype, {
    clientHeight: {value: 0, writable: true},
    clientLeft: {value: 0, writable: true},
    clientTop: {value: 0, writable: true},
    clientWidth: {value: 0, writable: true},
    focusTick_: {value: 0, writable: true},
    handleEvent: {value: handleEvent},
    status: {get: statusGet, set: statusSet},
    status_: {value: '', writable: true},
  });
})();
