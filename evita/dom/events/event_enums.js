// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
'use strict';

/** @enum{string} */
Event.Names = {
  ATTACH: 'attach', // DocumentEvent
  BEFORELOAD: 'beforeload',
  BLUR: 'blur',
  CHANGE: 'change',
  CLICK: 'click',
  DBLCLICK: 'dblclick',
  DETACH: 'detach', // DocumentEvent
  DROPWINDOW: 'dropwindow',
  FOCUS: 'focus',
  IDLE: 'idle',
  INPUT: 'input',
  KEYDOWN: 'keydown',
  KEYUP: 'keyup',
  LOAD: 'load',
  MOUSEDOWN: 'mousedown',
  MOUSEMOVE: 'mousemove',
  MOUSEUP: 'mouseup',
  QUERYCLOSE: 'queryclose',
  SAVE: 'save',
  WHEEL: 'wheel'
};

/** @enum */
Event.PhaseType = {
  NONE: 0,
  CAPTURING_PHASE: 1,
  AT_TARGET: 2,
  BUBBLING_PHASE: 3
};
