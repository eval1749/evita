// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
'use strict';

/** @enum{string} */
Event.Names = {
  ATTACH: 'attach',  // TextDocumentEvent
  BEFORELOAD: 'beforeload',
  BLUR: 'blur',
  CHANGE: 'change',
  CLICK: 'click',
  COMPOSITIONCANCEL: 'compositioncancel',
  COMPOSITIONCOMMIT: 'compositioncommit',
  COMPOSITIONEND: 'compositionend',
  COMPOSITIONSTART: 'compositionstart',
  COMPOSITIONUPDATE: 'compositionupdate',
  DBLCLICK: 'dblclick',
  DESTROY: 'destroy',
  DETACH: 'detach',  // TextDocumentEvent
  DROPWINDOW: 'dropwindow',
  FOCUS: 'focus',
  HIDE: 'hide',
  INPUT: 'input',
  KEYDOWN: 'keydown',
  KEYUP: 'keyup',
  LOAD: 'load',
  MOUSEDOWN: 'mousedown',
  MOUSEENTER: 'mouseenter',
  MOUSELEAVE: 'mouseleave',
  MOUSEMOVE: 'mousemove',
  MOUSEUP: 'mouseup',
  NEWFILE: 'newfile',  // TextDocumentEvent
  QUERYCLOSE: 'queryclose',
  REALIZE: 'realize',
  REMOVE: 'remove',
  SAVE: 'save',
  SELECTIONCHANGE: 'selectionchange',
  SHOW: 'show',
  WHEEL: 'wheel'
};

/** @enum */
Event.PhaseType = {
  NONE: 0,
  CAPTURING_PHASE: 1,
  AT_TARGET: 2,
  BUBBLING_PHASE: 3
};
