// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Object.defineProperty(/** @type {!Object} */(global), 'console', (function() {
  'use strict';

  const DOCUMENT_NAME = '*javascript*';

  ////////////////////////////////////////////////////////////
  //
  // Console
  //
  class Console {
    /**
     * @param {*} expression
     * @param {...} args
     */
    assert(expression, args) {
      if (expression)
        return;
      if (arguments.length === 1)
        throw new Error('Assertion failed');
      let message = Array.prototype.slice.call(arguments, 1).map(function(arg) {
        if (typeof(arg) === 'string')
          return arg;
        return repl.stringify(arg);
      }).join(' ');
      throw new Error('Assertion failed: ' + message);
    }

    /*
     * Clear console log contents.
     */
    clear() {
      let document = this.ensureDocument();
      let range = new Range(document, 0, document.length);
      range.text = '';
      console.update();
    }

    /** @return {!Document} */
    ensureDocument() {
      let present = Document.find(DOCUMENT_NAME);
      if (present)
        return present;
      let document = Document.new(DOCUMENT_NAME);
      document.mode = Mode.chooseModeByFileName('foo.js');
      return document;
    }

    // Output arguments to console log.
    /**
     * @param {...} varArgs
     */
    log(varArgs) {
      let message = Array.prototype.slice.call(arguments, 0).map(function(arg) {
        try {
          if (typeof(arg) === 'string')
            return arg;
          return repl.stringify(arg);
        } catch (e) {
          return repl.stringify(e);
        }
      }).join(' ');
      let document = this.ensureDocument();
      let range = new Range(document);
      range.collapseTo(document.length);
      let start = range.start;
      range.startOf(Unit.LINE, Alter.EXTEND);
      if (start !== range.start) {
        range.collapseTo(range.end);
        message = '\n' + message;
      }
      let readonly = document.readonly;
      document.readonly = false;
      range.insertBefore(message + '\n');
      document.readonly = readonly;
      console.update();
    }

    // Active or create window to show console log.
    show() {
      let document = this.ensureDocument();
      let windows = document.listWindows();
      if (windows.length) {
        windows[0].focus();
        return;
      }
      let editorWindow = new EditorWindow();
      let window = new TextWindow(new Range(document));
      editorWindow.appendChild(window);
      editorWindow.realize();
    }

    update() {
      for (let window of this.ensureDocument().listWindows()) {
        window.update();
      }
    }
  }
  Object.seal(Console.prototype);
  Object.seal(Console);

  let console = new Console();
  // TODO(eval1749): Once closure compiler supports getter in class, we should
  // use getter syntax.
  Object.defineProperties(console, {
    document: { get: function() { return this.ensureDocument(); } }
  });
  Object.seal(console);

  return {value: console};
})())
