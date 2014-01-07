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

  editor.setKeyBinding = function(combination, command) {
    editor.setKeyBinding_(parseKeyCombination(combination), command);
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

  // Select all contents
  editor.setKeyBinding('Ctrl+A', function() {
    var range = this.selection.range;
    range.start = 0
    range.end = range.document.length;
    this.selection.startIsActive = false;
  });

  // Open new document in new window in current editor window.
  editor.setKeyBinding('Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arg !== undefined) {
      newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    editor.getFilenameForSave(this, this.selection.document.filename || '',
                              function(filename) {
      newTextWindow(editorWindow, new Document(filename));
    });
  });

  // Open new document in new editor window.
  editor.setKeyBinding('Ctrl+Shift+N', function(arg) {
    if (arg !== undefined) {
      newEditorWindow(new Document('untitled.txt'));
      return;
    }

    editor.getFilenameForSave(this, this.selection.document.filename || '',
        function(filename) {
          newEditorWindow(new Document(filename));
        });
  });

  // Open document in new or existing window in current editor window.
  editor.setKeyBinding('Ctrl+O', function() {
    var editorWindow = this.parent;
    editor.getFilenameForLoad(this, this.selection.document.filename,
                              function(filename) {
      // TODO(yosi) When editorWindow has a window for filename, we should
      // activate it.
      newTextWindow(editorWindow, Document.load(filename));
    });
  });

  // Open document in new editor window.
  editor.setKeyBinding('Ctrl+Shift+O', function() {
    editor.getFilenameForLoad(this, this.selection.document.filename,
                              function(filename) {
      newEditorWindow(Document.load(filename));
    });
  });
})();
