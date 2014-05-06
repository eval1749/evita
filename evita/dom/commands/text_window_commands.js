// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /** @param {!Range} range */
  function copyToClipboard(range) {
    var items = DataTransfer.clipboard.items;
    items.clear();
    items.add(range.text.replace(/\n/g, '\r\n'), 'text/plain');
  }

  /** @this {!TextWindow} window */
  function copyToClipboardCommand() {
    copyToClipboard(this.selection.range);
  }

  /** @this {!TextWindow} window */
  function cutToClipboardCommand() {
    var range = this.selection.range;
    copyToClipboard(range);
    range.text = '';
  }

  /**
   * @param {!Unit} unit
   * @param {number} direction
   */
  function makeSelectionMotionCommand(unit, direction, alter) {
    /**
     * @this {!TextWindow}
     * @param {number=} opt_count
     */
    return function(opt_count) {
      var count = arguments.length >= 1 ? /** @type {number} */(opt_count) : 1;
      this.selection.modify(unit, count * direction, alter);
    };
  }

  /**
   * @param {number} charCode
   */
  function makeTypeCharCommand(charCode) {
    return charCode == 0x29 || charCode == 0x5D || charCode == 0x7D ?
      function(opt_count) {
        if (arguments.length)
          typeRightBracket.call(this, charCode, opt_count);
        else
          typeRightBracket.call(this, charCode);
      } : function(opt_count) {
        if (arguments.length)
          typeChar.call(this, charCode, opt_count);
        else
          typeChar.call(this, charCode);
      };
  }

  /**
   * Paste from clipboard
   * @this {!TextWindow}
   */
  function pasteFromClipboardCommand() {
    var items = DataTransfer.clipboard.items;
    if (!items.length)
      return;
    var item = items.get(0);
    if (item.kind != 'string')
      return;
    var range = this.selection.range;
    range.text = item.getAsString().replace(/\r\n/g, '\n');
    range.collapseTo(range.end);
  }

  /**
   * Type character
   * @this {!TextWindow}
   * @param {number} charCode
   * @param {number=} opt_count
   */
  function typeChar(charCode, opt_count) {
    var count = arguments.length >= 2 ? /**@type{number}*/(opt_count) : 1;
    var range = this.selection.range;
    range.text = String.fromCharCode(charCode).repeat(count);
    range.collapseTo(range.end);
  }

  /**
   * Type right bracket.
   * @this {!TextWindow}
   * @param {number} charCode
   * @param {number=} opt_count
   */
  function typeRightBracket(charCode, opt_count) {
    var count = arguments.length >= 2 ? /**@type{number}*/(opt_count) : 1;
    var window = this;
    var selection = window.selection;
    var range = selection.range;
    range.text = String.fromCharCode(charCode).repeat(count);
    range.collapseTo(range.end);
    var start = range.start;
    var end = range.end;

    // TODO(yosi) Should we move matching bracket blinking into |TextWindow|
    // idle processing?
    new OneShotTimer().start(0, function() {
      // Force color newly inserted characters.
      // Note: If we are in long comment, parenthesis matching may not work.
      range.document.doColor_(100);
      selection.move(Unit.BRACKET, -1);
      if (range.start == start) {
        range.collapseTo(end);
        selection.window.status = Strings.IDS_NO_MATCHING_PAREN;
        return;
      }

      // Move caret to left bracket 100ms or 500ms if left bracket above window.
      range.collapseTo(range.start);
      selection.document.readonly = true;
      // TODO(yosi) Should we share blink timer?
      var windowStart = window.compute_(TextWindowComputeMethod.MOVE_WINDOW,
                                       0, -1);
      new OneShotTimer().start(range.start < windowStart ? 500 : 100,
          function() {
            range.collapseTo(end);
            range.document.readonly = false;
          });
    });
  }

  // Install |TypeChar| commands.
  for (var charCode = 0x20; charCode < 0x7F; ++charCode) {
    Editor.bindKey(TextWindow, String.fromCharCode(charCode),
        makeTypeCharCommand(charCode),
        'type character ' + String.fromCharCode(charCode));
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
   * Reeconvert using IME.
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+,', function() {
    var range = this.selection.range;
    this.reconvert_(range.start, range.end);
  }, 'Reconvert');

  /**
   * Exchange Unicode character code point and character
   * @this {!TextWindow}
   * @param {number=} opt_base
   */
  Editor.bindKey(TextWindow, 'Ctrl+.', function(opt_base) {
    var base = arguments.length >= 1 ? /** @type{number} */(opt_base) : 16;
    if (base < 2 || base > 36)
      return;
    var range = new Range(this.selection.range);
    if (range.start == range.end) {
      range.moveStart(Unit.WORD, -1);
      if (range.end - range.start > 4)
        range.start = range.end - 4;
    } else if (range.end - range.start > 4) {
      return;
    }
    var text = range.text;
    if (!text.length)
      return;
    /** @type {function(number, number): number} */
    var fromDigitChar = function(charCode, base) {
      if (charCode >= Unicode.DIGIT_ZERO &&
          charCode <= Unicode.DIGIT_ZERO + Math.min(base, 9)) {
        return charCode - Unicode.DIGIT_ZERO;
      }

      if (base < 10)
        return NaN;

      if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
          charCode <= Unicode.LATIN_CAPITAL_LETTER_A + base - 10) {
        return charCode - Unicode.LATIN_CAPITAL_LETTER_A + 10;
      }

      if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
          charCode <= Unicode.LATIN_SMALL_LETTER_A + base - 10) {
        return charCode - Unicode.LATIN_SMALL_LETTER_A + 10;
      }

      return NaN;
    };

    /** @type {function(string, number): number} */
    var myParseInt = function(text, base) {
      var value = 0;
      for (var k = 0; k < text.length; ++k) {
        var digit = fromDigitChar(text.charCodeAt(k), base);
        if (isNaN(digit))
          return NaN;
        value *= base;
        value += digit;
      }
      return value;
    };
    var charCode = myParseInt(text, base);
    if (isNaN(charCode)) {
      range.start = range.end - 1;
      range.text = ('000' + text.charCodeAt(text.length - 1).toString(16))
          .substr(-4);
    } else {
      range.text = String.fromCharCode(charCode);
    }
    this.selection.range.collapseTo(range.end);
  }, 'Exchange Unicode code point and character\n');

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

  Editor.bindKey(TextWindow, 'Ctrl+ArrowLeft',
    makeSelectionMotionCommand(Unit.WORD, -1, Alter.MOVE),
    'move selection left word\n' +
    'Move selection to left by words');

  Editor.bindKey(TextWindow, 'Ctrl+ArrowRight',
    makeSelectionMotionCommand(Unit.WORD, 1, Alter.MOVE),
    'move selection right word\n' +
    'Move selection to right by words');

  Editor.bindKey(TextWindow, 'Ctrl+Q', commander.startQuote);

  /**
   * Reload document
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+R', function(arg) {
    var document = this.document;
    if (!document.needSave())
      return;
    var selection = this.selection;
    var offset = selection.range.start;
    Editor.messageBox(this,
        Editor.localizeText(Strings.IDS_ASK_RELOAD, {name: document.name}),
        MessageBox.ICONQUESTION | MessageBox.YESNO)
    .then(function(responseCode) {
      if (responseCode != DialogItemId.YES)
        return;
      document.load().then(function() {
        selection.range.collapseTo(offset);
      });
    });
  }, 'reload document');

  /**
   * Save file
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+S', function(arg) {
    var document = this.selection.document;
    if (!arg && document.fileName != '') {
      document.save(document.fileName);
      return;
    }

    Editor.getFileNameForSave(this, document.fileName)
        .then(function(fileName) {
          document.save(fileName);
        });
  });

  Editor.bindKey(TextWindow, 'Ctrl+U', commander.startArgument);

  /**
   * Show document in new editor window.
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Ctrl+Shift+3', function() {
    var textWindow = new TextWindow(this.selection.range);
    var editorWindow = new EditorWindow();
    editorWindow.appendChild(textWindow);
    editorWindow.realize();
    textWindow.makeSelectionVisible();
    textWindow.focus();
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
    var textWindow = new TextWindow(this.selection.range);
    var editorWindow = new EditorWindow();
    editorWindow.appendChild(textWindow);
    editorWindow.realize();
    textWindow.makeSelectionVisible();
    textWindow.focus();
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

  /**
   * Evaluate script in selection selection
   * @this {!TextWindow}
   */
  function evalSelectionCommand() {
    var selection = this.selection;
    var isWhole = selection.range.start == selection.range.end;
    var scriptText = isWhole ?
        (new Range(this.document, 0, this.document.length)).text :
        selection.range.text;
    var result = Editor.runScript(scriptText, this.document.name);
    if (!result.exception) {
      if (result.value != undefined)
        console.log(result.value);
      return;
    }
    Editor.messageBox(this, result.stackTraceString, MessageBox.ICONERROR,
                      'Evaluate Selection Command')
      .then(function(x) {
        var offset = isWhole ? 0 : this.selection.range.start;
        selection.range.collapseTo(offset + result.start)
          .moveEnd(Unit.CHARACTER, result.end - result.start);
      });
  }
  Editor.bindKey(TextWindow, 'Ctrl+Shift+E', evalSelectionCommand,
    'Evaluate script in selection');
  Editor.bindKey(TextWindow, 'F5', evalSelectionCommand,
    'Evaluate script in selection');

  Editor.bindKey(TextWindow, 'Ctrl+Shift+Delete', copyToClipboardCommand);

  Editor.bindKey(TextWindow, 'Ctrl+Shift+End', function() {
    this.selection.endKey(Unit.DOCUMENT, Alter.EXTEND);
  }, 'extend to end of document\n' +
     'Move active position of selection to end of document.');

  Editor.bindKey(TextWindow, 'Ctrl+Shift+Home', function() {
    this.selection.homeKey(Unit.DOCUMENT, Alter.EXTEND);
  }, 'exthome to home of document\n' +
     'Move active position of selection to home of document.');

  Editor.bindKey(TextWindow, 'Ctrl+Shift+ArrowLeft',
    makeSelectionMotionCommand(Unit.WORD, -1, Alter.EXTEND),
    'extend selection left word\n' +
    'Extend selection to left by words');

  Editor.bindKey(TextWindow, 'Ctrl+Shift+ArrowRight',
    makeSelectionMotionCommand(Unit.WORD, 1, Alter.EXTEND),
    'extend selection right word\n' +
    'Extend selection to right by words');

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
    var position = selection.document.redo(selection.focusOffset);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_REDO),
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
    var position = selection.document.undo(selection.focusOffset);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_UNDO),
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

  /**
   * @param {number=} opt_count
   * @this {!TextWindow}
   */
  Editor.bindKey(TextWindow, 'Enter', function(opt_count) {
    var count = arguments.length >= 1 ? /** @type {number} */(opt_count) : 1;
    if (count <= 0)
      return;
    var selection = this.selection;
    var leadingWhitespaces = (new Range(selection.range)).startOf(Unit.LINE)
        .moveEndWhile(' \t').text;
    selection.document.undoGroup('TypeEnter', function() {
      selection.range.moveStartWhile(' \t', Count.BACKWARD);
      selection.range.text = '\n'.repeat(count);
      selection.range.collapseTo(selection.range.end)
          .moveEndWhile(' \t').text = leadingWhitespaces;
      selection.range.collapseTo(selection.range.end);
    });
  }, 'type enter key\n' +
     'Insert newlines with leading whitespaces and remove trailing' +
     ' whitespaces.');

  Editor.bindKey(TextWindow, 'Home', function() {
    this.selection.homeKey(Unit.WINDOW_LINE);
  }, 'move to home of window line\n' +
     'Move active position of selection to home of window line.');

  Editor.bindKey(TextWindow, 'ArrowLeft',
      makeSelectionMotionCommand(Unit.CHARACTER, -1, Alter.MOVE),
      'move selection to left by character');

  Editor.bindKey(TextWindow, 'ArrowRight',
      makeSelectionMotionCommand(Unit.CHARACTER, 1, Alter.MOVE),
      'move selection to right by character');

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

  Editor.bindKey(TextWindow, 'Shift+ArrowLeft',
      makeSelectionMotionCommand(Unit.CHARACTER, -1, Alter.EXTEND),
      'extend selection to left by character');

  Editor.bindKey(TextWindow, 'Shift+ArrowRight',
      makeSelectionMotionCommand(Unit.CHARACTER, 1, Alter.EXTEND),
      'extend selection to right by character');

  /** @const @type {number} */
  // TODO(yosi) We should get |Indent|/|Outend| tab width from another place.
  var TAB_WIDTH = 4;

  /** @param {number=} opt_count */
  Editor.bindKey(TextWindow, 'Shift+Tab', function(opt_count) {
    var tabWidth = arguments.length >= 1 ? /** @type{number} */(opt_count) :
                                          TAB_WIDTH;
    var range = this.selection.range;
    if (range.start == range.end) {
      // Move to previous tab stop
      range.startOf(Unit.LINE, Alter.EXTEND);
      range.collapseTo(Math.floor((range.start - range.end - 1) / tabWidth) *
                       tabWidth + range.end);
      return;
    }
    // Exapnd range to contain whole lines
    range.startOf(Unit.LINE, Alter.EXTEND);
    // Remove leading whitespaces
    range.document.undoGroup('Outdent', function() {
      var document = this;
      var lineRange = new Range(range);
      lineRange.collapseTo(lineRange.start);
      while (lineRange.start < range.end) {
        lineRange.moveEndWhile(' ', tabWidth);
        if (lineRange.end < document.length &&
            lineRange.end - lineRange.start < tabWidth &&
            document.charCodeAt_(lineRange.end) == 0x09) {
          lineRange.moveEnd(Unit.CHARACTER);
        }
        lineRange.text = '';
        lineRange.endOf(Unit.LINE).move(Unit.CHARACTER);
      }
      range.end = lineRange.start;
    });
  }, 'outdent\n' +
     'Remove leading whitespaces from lines in selection.');

  /** @param {number=} opt_count */
  Editor.bindKey(TextWindow, 'Tab', function(opt_count) {
    var tabWidth = arguments.length >= 1 ? /** @type{number} */(opt_count) :
                                          TAB_WIDTH;
    var range = this.selection.range;
    if (range.start == range.end) {
      var current = range.start;
      range.startOf(Unit.LINE, Alter.EXTEND);
      var numSpaces = tabWidth - (current - range.start) % tabWidth;
      range.start = range.end;
      range.insertBefore(' '.repeat(numSpaces));
      return;
    }

    // Exapnd range to contain whole lines
    range.startOf(Unit.LINE, Alter.EXTEND);
    range.document.undoGroup('Indent', function() {
      var lineRange = new Range(range);
      lineRange.collapseTo(lineRange.start);
      while (lineRange.start < range.end) {
        var spaces = ' '.repeat(tabWidth);
        lineRange.endOf(Unit.LINE, Alter.EXTEND);
        if (lineRange.start != lineRange.end)
          lineRange.insertBefore(spaces);
        lineRange.move(Unit.CHARACTER);
      }
      range.end = lineRange.start;
    });
    // Make selection contains spaces inserted at first line.
    range.startOf(Unit.LINE, Alter.EXTEND);
    this.selection.startIsActive = false;
  }, 'indent\n' +
     'Caret: insert spaces until tab stop column.\n' +
     'Range: insert spaces all lines in range.');

  Editor.bindKey(TextWindow, 'ArrowUp',
      makeSelectionMotionCommand(Unit.WINDOW_LINE, -1, Alter.MOVE),
      'move up by window line\n' +
      'Move focus position up by window line.');

  Editor.bindKey(TextWindow, 'ArrowDown',
      makeSelectionMotionCommand(Unit.WINDOW_LINE, 1, Alter.MOVE),
      'move down by window line\n' +
      'Move focus position down by window line.');

  Editor.bindKey(TextWindow, 'PageUp',
      makeSelectionMotionCommand(Unit.SCREEN, -1, Alter.MOVE),
      'move up by screen\n' +
      'Move focus position up by screen.');

  Editor.bindKey(TextWindow, 'PageDown',
      makeSelectionMotionCommand(Unit.SCREEN, 1, Alter.MOVE),
      'move down by screen\n' +
      'Move focus position down by screen.');

  Editor.bindKey(TextWindow, 'Ctrl+ArrowUp',
      makeSelectionMotionCommand(Unit.BRACKET, -1, Alter.MOVE),
      'move up by bracket\n' +
      'Move focus position up by bracket.');

  Editor.bindKey(TextWindow, 'Ctrl+ArrowDown',
      makeSelectionMotionCommand(Unit.BRACKET, 1, Alter.MOVE),
      'move down by bracket\n' +
      'Move focus position down by bracket.');

  Editor.bindKey(TextWindow, 'Ctrl+PageUp',
      makeSelectionMotionCommand(Unit.WINDOW, -1, Alter.MOVE),
      'move up by window\n' +
      'Move focus position up by window.');

  Editor.bindKey(TextWindow, 'Ctrl+PageDown',
      makeSelectionMotionCommand(Unit.WINDOW, 1, Alter.MOVE),
      'move down by window\n' +
      'Move focus position down by window.');

  Editor.bindKey(TextWindow, 'Shift+ArrowUp',
      makeSelectionMotionCommand(Unit.WINDOW_LINE, -1, Alter.EXTEND),
      'extend up by window line\n' +
      'Extend focus position up by window line.');

  Editor.bindKey(TextWindow, 'Shift+ArrowDown',
      makeSelectionMotionCommand(Unit.WINDOW_LINE, 1, Alter.EXTEND),
      'extend down by window line\n' +
      'Extend focus position down by window line.');

  Editor.bindKey(TextWindow, 'Shift+PageUp',
      makeSelectionMotionCommand(Unit.SCREEN, -1, Alter.EXTEND),
      'extend up by screen\n' +
      'Extend focus position up by screen.');

  Editor.bindKey(TextWindow, 'Shift+PageDown',
      makeSelectionMotionCommand(Unit.SCREEN, 1, Alter.EXTEND),
      'extend down by screen\n' +
      'Extend focus position down by screen.');

  Editor.bindKey(TextWindow, 'Shift+Ctrl+ArrowDown',
      makeSelectionMotionCommand(Unit.BRACKET, 1, Alter.EXTEND),
      'extend down by bracket\n' +
      'Extend focus position down by bracket');

  Editor.bindKey(TextWindow, 'Shift+Ctrl+ArrowUp',
      makeSelectionMotionCommand(Unit.BRACKET, -1, Alter.EXTEND),
      'extend up by bracket\n' +
      'Extend focus position up by bracket');

  Editor.bindKey(TextWindow, 'Shift+Ctrl+PageUp',
      makeSelectionMotionCommand(Unit.WINDOW, -1, Alter.EXTEND),
      'extend up by window\n' +
      'Extend focus position up by window.');

  Editor.bindKey(TextWindow, 'Shift+Ctrl+PageDown',
      makeSelectionMotionCommand(Unit.WINDOW, 1, Alter.EXTEND),
      'extend down by window\n' +
      'Extend focus position down by window.');
})();
