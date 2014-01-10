// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /** @const */ var MOD_CTRL = 0x200;
  /** @const */ var MOD_SHIFT = 0x400;

  var KEY_COMBINATION_RULES = [
    {
      modifiers: MOD_CTRL | MOD_SHIFT,
      re: /^Ctrl[-+]Shift[-+](.+)$/i
    },
    {
      modifiers: MOD_CTRL | MOD_SHIFT,
      re: /^Shift[-+]Ctrl[-+](.+)$/i
    },
    {
      modifiers: MOD_CTRL,
      re: /^Ctrl[-+](.+)$/i
    },
    {
      modifiers: MOD_SHIFT,
      re: /^Shift[-+](.+)$/i
    },
    {
      modifiers: 0,
      re: /^(.+)$/
    }
  ];

  function parseKeyCombination(spec) {
    var code = 0;
    KEY_COMBINATION_RULES.forEach(function(rule) {
      if (code)
        return;
      var matches = rule.re.exec(spec);
      if (!matches)
        return;
      var key_name = matches[1];
      var key_code = KEY_CODE_MAP[key_name.toLowerCase()];
      if (!key_code)
        throw new Error('Invalid key name: ' + key_name);
      code = rule.modifiers | key_code;
    });
    if (!code)
      throw new Error('Invalid key specification: ' + spec);
    return code;
  }

  Editor.setKeyBinding = function(combination, command) {
    Editor.setKeyBinding_(parseKeyCombination(combination), command);
  };
})();

(function() {
  /**
   * @param {Document} document.
   */
  function newEditorWindow(document) {
    var editorWindow = new EditorWindow();
    newTextWindow(editorWindow, document);
    editorWindow.realize();
  }

  /**
   * @param {Window} document.
   * @param {Document} document.
   */
  function newTextWindow(parent, document) {
    parent.add(new TextWindow(new Range(document)));
  }

  // Show document in new editor window.
  Editor.setKeyBinding('Ctrl+Shift+3', function() {
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.add(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
  });

  // Show document in new editor window and close current editor window.
  Editor.setKeyBinding('Ctrl+Shift+4', function() {
    var current = this.selection.window.parent;
    // When editor window has only one tab, we don't ignore this command,
    // since result of this command isn't useful.
    if (current.children.length == 1)
      return;
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.add(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
    current.destroy();
  });

  // Select all contents
  Editor.setKeyBinding('Ctrl+A', function() {
    var range = this.selection.range;
    range.start = 0
    range.end = range.document.length;
    this.selection.startIsActive = false;
  });

  // Make selection visible
  Editor.setKeyBinding('Ctrl+L', function() {
    this.makeSelectionVisible();
  });

  // Open new document in new window in current editor window.
  Editor.setKeyBinding('Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arg !== undefined) {
      newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newTextWindow(editorWindow, new Document(filename))
        });
  });

  // Open new document in new editor window.
  Editor.setKeyBinding('Ctrl+Shift+N', function(arg) {
    if (arg !== undefined) {
      newEditorWindow(new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(new Document(filename));
        });
  });

  // Open document in new or existing window in current editor window.
  Editor.setKeyBinding('Ctrl+O', function() {
    var editorWindow = this.parent;
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          // TODO(yosi) When editorWindow has a window for filename, we should
          // activate it.
          newTextWindow(editorWindow, Document.load(filename));
        });
  });

  // Open document in new editor window.
  Editor.setKeyBinding('Ctrl+Shift+O', function() {
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(Document.load(filename));
        });
  });

  // Save file
  Editor.setKeyBinding('Ctrl+S', function(arg) {
    var document = this.selection.document;
    if (!arg && document.filename != '') {
      document.save(filename);
      return;
    }

    Editor.getFilenameForSave(this, document.filename)
        .then(function(filename) {
          document.save(filename);
        });
  });

  Editor.setKeyBinding('Ctrl+W', function(arg) {
    var document = this.selection.document;
    document.close();
  });
})();
