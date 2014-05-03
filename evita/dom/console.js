// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.console = new Object();

Object.defineProperties(console, {
  DOCUMENT_NAME: {
    value: '*javascript*'
  },

  assert: {value:
    /**
     * @param {*} expression
     * @param {*=} opt_message
     */
    function(expression, opt_message) {
      if (expression)
        return;
      if (arguments.length >= 2)
        throw new Error('Assertion failed: ' + opt_message);
      throw new Error('Assertion failed');
    }
  },

  /*
   * Clear console log contents.
   */
  clear: {
    value: function() {
      var document = console.document;
      var range = new Range(document, 0, document.length);
      range.text = '';
      console.update();
    }
  },

  document: {
    /** @type {!function(): !Document} */
    get: function() {
      var present = Document.find(console.DOCUMENT_NAME);
      if (present)
        return present;
      var document = new Document(console.DOCUMENT_NAME);
      document.mode = new JavaScriptMode();
      return document;
    }
  },

  // Output arguments to console log.
  log: {
    /**
     * @param {...} var_args
     */
    value: function(var_args) {
      var message = Array.prototype.slice.call(arguments, 0).map(function(arg) {
        try {
          if (typeof(arg) == 'string')
            return arg;
          return Editor.stringify(arg);
        } catch (e) {
          return Editor.stringify(e);
        }
      }).join(' ');
      var document = console.document;
      var range = new Range(document);
      range.collapseTo(document.length);
      var start = range.start;
      range.startOf(Unit.LINE, Alter.EXTEND);
      if (start != range.start) {
        range.collapseTo(range.end);
        message = '\n' + message;
      }
      var readonly = document.readonly;
      document.readonly = false;
      range.insertBefore(message + '\n');
      document.readonly = readonly;
      console.update();
    }
  },

  // Active or create window to show console log.
  show: {
    value: function() {
      var document = console.document;
      var windows = document.listWindows();
      if (windows.length) {
        windows[0].focus();
        return;
      }
      var editorWindow = new EditorWindow();
      var window = new TextWindow(new Range(console.document));
      editorWindow.appendChild(window);
      editorWindow.realize();
    }
  },

  update: {
    value: function() {
      console.document.listWindows().forEach(function(window) {
        window.update();
      });
    }
  }
});
