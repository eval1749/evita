// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

/** @type {string} */
Window.prototype.status_;

/** @type {boolean} */
Window.prototype.visible_;

// See "//evita/dom/global/commander.js" for |Window.handleKeyboardEvent()|.

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
      return Window.handleKeyboardEvent(/** @type {!KeyboardEvent} */ (event));
    case Event.Names.SHOW:
      this.visible_ = true;
      return;
  }
}

/**
 * @this {!Window}
 * @return {string}
 */
function statusGet() {
  return this.status_;
}

/**
 * @this {!Window}
 */
function statusSet(newStatus) {
  if (this.status_ === newStatus)
    return;
  this.status_ = newStatus;
  Editor.messageBox(this, this.status_, MessageBox.ICONINFORMATION);
}

/**
 * @this {!Window}
 * @return {boolean}
 */
function getVisible() {
  return this.visible_;
}

Object.defineProperties(Window, {handleEvent: {value: handleEvent}});

Object.defineProperties(Window.prototype, {
  focusTick_: {value: 0, writable: true},
  status: {get: statusGet, set: statusSet},
  status_: {value: '', writable: true},
  visible: {get: getVisible},
  visible_: {value: false, writable: true}
});
});
