// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /** @this {!TextWindow} window */
  function copyToClipboardCommand() {
    this.selection.range.copy();
  }

  /** @this {!TextWindow} window */
  function cutToClipboardCommand() {
    this.selection.range.copy();
    this.selection.range.text = '';
  }

  /**
   * Paste from clipboard
   * @this {!TextWindow}
   */
  function pasteFromClipboardCommand() {
    this.selection.range.paste();
  }

  /**
   * Backward delete character
   * @param {number=} opt_count
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Backspace', function(opt_count) {
    var count = arguments.length >= 1 ? opt_count : 1;
    this.selection.range.delete(Unit.CHARACTER, -count);
  });

  /**
   * Select all contents
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+A', function() {
    var range = this.selection.range;
    range.start = 0
    range.end = range.document.length;
    this.selection.startIsActive = false;
  });

  /**
   * Backward delete word
   * @param {number=} opt_count
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Backspace', function(opt_count) {
    var count = arguments.length >= 1 ? opt_count : 1;
    this.selection.range.delete(Unit.WORD, -count);
  });

  Editor.bindKey(TextWindow, 'Ctrl+C', copyToClipboardCommand);

  /**
   * Forward delete word
   * @param {number=} opt_count
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Delete', function(opt_count) {
    var count = arguments.length >= 1 ? opt_count : 1;
    this.selection.range.delete(Unit.WORD, count);
  });

  Editor.bindKey(TextWindow, 'Ctrl+End', function() {
    this.selection.endKey(Unit.DOCUMENT);
  }, 'move to end of document\n' +
     'Move active position of selection to end of document.');

  /**
   * @param {TextSelection} selection
   * @param {function(TextSelection)} changer
   */
  function changeCase(selection, changer) {
    if (selection.start == selection.end) {
      selection.range.startOf(Unit.WORD, Alter.EXTEND);
      selection.range.endOf(Unit.WORD, Alter.EXTEND);
    }
    changer(selection);
  }

  // TODO(yosi) We should display dialog box to prompt enter line number and
  // list of functions.
  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+G', function(arg) {
    if (!arg)
      return;
    this.selection.range.startOf(Unit.DOCUMENT);
    this.selection.range.move(Unit.LINE, arg - 1);
  });

  Editor.bindKey(TextWindow, 'Ctrl+Home', function() {
    this.selection.homeKey(Unit.DOCUMENT);
  }, 'move to home of document\n' +
     'Move active position of selection to home of document.');

  /**
   * Make slection visible.
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+L', function() {
    this.makeSelectionVisible();
  });

  /**
   * Save file
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+S', function(arg) {
    var document = this.selection.document;
    if (!arg && document.filename != '') {
      document.save(document.filename);
      return;
    }

    Editor.getFilenameForSave(this, document.filename)
        .then(function(filename) {
          document.save(filename);
        });
  });

  /**
   * Show document in new editor window.
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+3', function() {
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.appendChild(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
  });

  /**
   * Show document in new editor window and close current editor window.
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+4', function() {
    var current = this.selection.window.parent;
    // When editor window has only one tab, we don't ignore this command,
    // since result of this command isn't useful.
    if (current.children.length == 1)
      return;
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.appendChild(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
    current.destroy();
  });

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+C', function() {
    changeCase(this.selection, function(selection) {
      selection.range.capitalize();
    });
  });

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+D', function() {
    changeCase(this.selection, function(selection) {
      selection.range.toLowerCase();
    });
  });

  Editor.bindKey(TextWindow, 'Ctrl+Shift+Delete', copyToClipboardCommand);

  Editor.bindKey(TextWindow, 'Ctrl+Shift+End', function() {
    this.selection.endKey(Unit.DOCUMENT, Alter.EXTEND);
  }, 'extend to end of document\n' +
     'Move active position of selection to end of document.');

  Editor.bindKey(TextWindow, 'Ctrl+Shift+Home', function() {
    this.selection.homeKey(Unit.DOCUMENT, Alter.EXTEND);
  }, 'exthome to home of document\n' +
     'Move active position of selection to home of document.');

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+U', function() {
    changeCase(this.selection, function(selection) {
      selection.range.toUpperCase();
    });
  });

  Editor.bindKey(TextWindow, 'Ctrl+V', pasteFromClipboardCommand);

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+W', function(arg) {
    var document = this.selection.document;
    document.close();
  });

  Editor.bindKey(TextWindow, 'Ctrl+X', cutToClipboardCommand);

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Y', function() {
    var selection = this.selection;
    var position = selection.document.redo(selection.active);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_REDO),
          Editor.localizeText(Strings.IDS_APP_TITLE),
          MessageBox.ICONWARNING);
      return;
    }
    selection.range.collapseTo(position);
  });

  /**
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Z', function() {
    var selection = this.selection;
    var position = selection.document.undo(selection.active);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_UNDO),
          Editor.localizeText(Strings.IDS_APP_TITLE),
          MessageBox.ICONWARNING);
      return;
    }
    selection.range.collapseTo(position);
  });

  /**
   * Forward delete character
   * @param {number=} opt_count
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Delete', function(opt_count) {
    var count = arguments.length >= 1 ? opt_count : 1;
    this.selection.range.delete(Unit.CHARACTER, count);
  });

  Editor.bindKey(TextWindow, 'End', function() {
    this.selection.endKey(Unit.WINDOW_LINE);
  }, 'move to end of window line\n' +
     'Move active position of selection to end of window line.');

  Editor.bindKey(TextWindow, 'Home', function() {
    this.selection.homeKey(Unit.WINDOW_LINE);
  }, 'move to home of window line\n' +
     'Move active position of selection to home of window line.');

  Editor.bindKey(TextWindow, 'Shift+Delete', cutToClipboardCommand);

  Editor.bindKey(TextWindow, 'Shift+End', function() {
    this.selection.endKey(Unit.WINDOW_LINE, Alter.EXTEND);
  }, 'extend to end of window line\n' +
     'Move active position of selection to end of window line.');

  Editor.bindKey(TextWindow, 'Shift+Home', function() {
    this.selection.homeKey(Unit.WINDOW_LINE, Alter.EXTEND);
  }, 'exthome to home of window line\n' +
     'Move active position of selection to home of window line.');

  Editor.bindKey(TextWindow, 'Shift+Insert', pasteFromClipboardCommand);
})();
