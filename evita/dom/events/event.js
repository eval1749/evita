// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
'use strict';

/** @enum{string} */
Event.Names = {
  BLUR: 'blur',
  CLICK: 'click',
  DBLCLICK: 'dblclick',
  DROPWINDOW: 'dropwindow',
  FOCUS: 'focus',
  KEYDOWN: 'keydown',
  KEYUP: 'keyup',
  MOUSEDOWN: 'mousedown',
  MOUSEMOVE: 'mousemove',
  MOUSEUP: 'mouseup',
  QUERYCLOSE: 'queryclose'
};

/** @enum */
Event.PhaseType = {
  NONE: 0,
  CAPTURING_PHASE: 1,
  AT_TARGET: 2,
  BUBBLING_PHASE: 3
};
