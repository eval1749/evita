// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.console = new Object();

Object.defineProperties(console, {
  DOCUMENT_NAME: {
    value: '*javascript*'
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
      var document = Document.find(console.DOCUMENT_NAME);
      if (document)
        return document;
      return new Document(console.DOCUMENT_NAME, new JavaScriptMode());
    }
  },

  // Output arguments to console log.
  log: {
    /**
     * @param {...} var_args
     */
    value: function(var_args) {
      function stringify(value) {
        switch (typeof(value)) {
          case 'string':
            return value;
          case 'symbol':
            return value.toString();
        }

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
