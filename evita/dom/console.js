// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @const @type{{
 *    DOCUMENT_NAME: string,
 *    clear: function(),
 *    document_: function(): !Document,
 *    log: function(...),
 *    show: function()
 * }}
 */
global.console = {
  /** @const @type{string} */
  DOCUMENT_NAME: '*javascript*',

  /**
   * Clear console log contents.
   */
  clear: function() {
    var document = console.document_();
    var range = new Range(document, 0, document.length);
    range.text = '';
  },

  /**
   * @private
   * @return {!Document}
   */
  document_: function() {
    return Document.getOrNew(console.DOCUMENT_NAME);
  },

  /**
   * Output arguments to console log.
   * @param {...} var_args
   */
  log: function(var_args) {
    function stringify(value) {
      if (typeof(value) == 'string')
        return value;

      if (JsConsole.stringify)
        return JsConsole.stringify(value);

      if (value === null)
        return 'null';
      if (value === undefined)
        return 'undefined';
      try {
        return value.toString();
      } catch (e) {
        return 'Exception in toString: ' + e;
      }
    }

    var message = Array.prototype.slice.call(arguments, 0).map(function(arg) {
      try {
        return stringify(arg);
      } catch (e) {
        return stringify(e);
      }
    }).join(' ');
    var document = console.document_();
    var range = new Range(document);
    range.collapseTo(document.length);
    var start = range.start;
    range.startOf(Unit.LINE, Alter.EXTEND);
    if (start != range.start) {
      range.collapseTo(range.end);
      message = '\n' + message;
    }
    range.insertBefore(message + '\n');
  },

  /**
   * Active or create window to show console log.
   */
  show: function() {
    var document = console.document_();
    var windows = document.listWindows();
    if (windows.length) {
      windows[0].focus();
      return;
    }
    var editorWindow = new EditorWindow();
    var window = new TextWindow(new Range(console.document_()));
    editorWindow.appendChild(window);
    editorWindow.realize();
  }
};
