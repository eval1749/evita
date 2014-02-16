// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  /**
   * Default event handler.
   * @this {!Window}
   * @param {!Event} event.
   */
  Window.handleEvent = function(event) {
    commander.handleEvent(event);
  }
})();

Object.defineProperty(Window.prototype, 'status', (function() {
  var status = '';
  return {
    /** @return {string} */
    get: function() {
      return status;
    },
    /** @param {string} new_status */
    set: function(new_status) {
      if (status == new_status)
        return;
      status = new_status;
      Editor.messageBox(this, status, MessageBox.ICONINFORMATION);
    }
  };
})());
