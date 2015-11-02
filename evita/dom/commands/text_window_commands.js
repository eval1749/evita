// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /** @const @type {Array.<number>} */
  var ZOOM_STEPS = [
    20, 30, 40, 50, 60, 70, 80, 90,
    100, 110, 130, 150, 170,
    200, 250,
    300, 350,
    400, 450,
    500,
  ];

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
    function command_function(opt_count) {
      var count = arguments.length >= 1 ? /** @type {number} */(opt_count) : 1;
      this.selection.modify(unit, count * direction, alter);
    }
    return command_function;
  }

  function isRightBracket(charCode) {
    // return charCode == 0x29 || charCode == 0x5D || charCode == 0x7D ?
    return false;
  }

  /**
   * @param {number} charCode
   */
  function makeTypeCharCommand(charCode) {
    return isRightBracket(charCode) ?
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

  /** @type {number}*/
  global.parenthesisColor = 0xCCFF90;

  /**
   * Type right bracket.
   * @this {!TextWindow}
   * @param {number} charCode
   * @param {number=} opt_count
   */
  function typeRightBracket(charCode, opt_count) {
    const count = /**@type{number}*/(opt_count) || 1;
    /** @type {!TextWindow} */
    const window = this;
    /** @type {!TextSelection} */
    const selection = /** @type {!TextSelection} */(window.selection);
    /** @type {!Document} */
    const document = selection.document;
    selection.range.text = String.fromCharCode(charCode).repeat(count);
    selection.range.collapseTo(selection.range.end);
    document.doColor_(1000);

    /** @type {!Range} */
    const enclosingRange = new Range(selection.range);
    enclosingRange.moveStart(Unit.BRACKET, -1);
    if (enclosingRange.collapsed) {
      window.status = Strings.IDS_NO_MATCHING_PAREN;
      return;
    }

    enclosingRange.setStyle({backgroundColor: global.parenthesisColor});
    new OneShotTimer().start(160, function() {
      // TODO(eval1749): We should restore original background color.
      enclosingRange.setStyle({backgroundColor: 0xFFFFFF});
    });
  }

  // Install |TypeChar| commands.
  for (var charCode = 0x20; charCode < 0x7F; ++charCode) {
    Editor.bindKey(TextWindow, String.fromCharCode(charCode),
        makeTypeCharCommand(charCode),
        'type character ' + String.fromCharCode(charCode));
  }

  Editor.bindKey(TextWindow, 'Backspace',
    /**
     * Backward delete character
     * @param {number=} opt_count
     * @this {!TextWindow}
     */
    function(opt_count) {
      var count = arguments.length >= 1 ? opt_count : 1;
      this.selection.range.delete(Unit.CHARACTER, -count);
    });

  Editor.bindKey(TextWindow, 'Ctrl++',
    /**
     * Zoom-In
     * @this {!TextWindow}
     */
    function() {
      var currentZoom = Math.round(this.zoom * 100);
      var index = ZOOM_STEPS.findIndex(function(zoom) {
        return zoom > currentZoom;
      });
      if (index >= 0)
        this.zoom = ZOOM_STEPS[index] / 100;
    }, 'Zoom In');

  Editor.bindKey(TextWindow, 'Ctrl+,',
    /**
     * Reconvert text using IME.
     * @this {!TextWindow}
     */
    function() {
      var range = this.selection.range;
      this.reconvert_(range.text);
      range.text = '';
    }, 'Reconvert');

  Editor.bindKey(TextWindow, 'Ctrl+-',
    /**
     * Zoom-Out
     * @this {!TextWindow}
     */
    function() {
      var currentZoom = Math.round(this.zoom * 100);
      var index = ZOOM_STEPS.findIndex(function(zoom) {
        return zoom >= currentZoom;
      });
      if (index >= 1)
        this.zoom = ZOOM_STEPS[index - 1] / 100;
    }, 'Zoom out');

  Editor.bindKey(TextWindow, 'Ctrl+.',
    /**
     * Exchange Unicode character code point and character
     * @this {!TextWindow}
     * @param {number=} opt_base
     */
    function(opt_base) {
      var base = arguments.length >= 1 ? /** @type{number} */(opt_base) : 16;
      if (base < 2 || base > 36)
        return;
      var range = new Range(this.selection.range);
      if (range.start == range.end) {
        range.moveStart(Unit.WORD, -1);
        if (range.length > 4)
          range.start = range.end - 4;
      } else if (range.length > 4) {
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

  Editor.bindKey(TextWindow, 'Ctrl+0',
    /**
     * Zoom-In
     * @this {!TextWindow}
     */
    function() {
      this.zoom = 1.0;
    });

  Editor.bindKey(TextWindow, 'Ctrl+A',
    /**
     * Select all contents
     * @this {!TextWindow}
     */
    function() {
      var range = this.selection.range;
      range.start = 0
      range.end = range.document.length;
      this.selection.startIsActive = false;
    });

  Editor.bindKey(TextWindow, 'Ctrl+Backspace',
    /**
     * Backward delete word
     * @param {number=} opt_count
     * @this {!TextWindow}
     */
    function(opt_count) {
      var count = arguments.length >= 1 ? opt_count : 1;
      this.selection.range.delete(Unit.WORD, -count);
    });

  Editor.bindKey(TextWindow, 'Ctrl+C', copyToClipboardCommand);

  Editor.bindKey(TextWindow, 'Ctrl+Delete',
    /**
     * Forward delete word
     * @param {number=} opt_count
     * @this {!TextWindow}
     */
    function(opt_count) {
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

  // TODO(eval1749): We should display dialog box to prompt enter line number and
  // list of functions.
  Editor.bindKey(TextWindow, 'Ctrl+G',
    /**
     * @this {!TextWindow}
     */
    function(arg) {
      if (!arg)
        return;
      this.selection.range.startOf(Unit.DOCUMENT);
      this.selection.range.move(Unit.LINE, arg - 1);
    });

  Editor.bindKey(TextWindow, 'Ctrl+Home', function() {
    this.selection.homeKey(Unit.DOCUMENT);
  }, 'move to home of document\n' +
     'Move active position of selection to home of document.');

  Editor.bindKey(TextWindow, 'Ctrl+L',
    /**
     * Make selection visible.
     * @this {!TextWindow}
     */
    function() {
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

  Editor.bindKey(TextWindow, 'Ctrl+R',
    /**
     * Reload document
     * @this {!TextWindow}
     */
    function(arg) {
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

  Editor.bindKey(TextWindow, 'Ctrl+S',
    /**
     * Save file
     * @this {!TextWindow}
     */
    function(arg) {
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

  Editor.bindKey(TextWindow, 'Ctrl+Shift+3',
    /**
     * Show document in new editor window.
     * @this {!TextWindow}
     */
    function() {
      var textWindow = new TextWindow(this.selection.range);
      var editorWindow = new EditorWindow();
      editorWindow.appendChild(textWindow);
      editorWindow.realize();
    });

  Editor.bindKey(TextWindow, 'Ctrl+Shift+4',
    /**
     * Show document in new editor window and close current editor window.
     * @this {!TextWindow}
     */
    function() {
      var current = this.selection.window.parent;
      // When editor window has only one tab, we don't ignore this command,
      // since result of this command isn't useful.
      if (current.children.length == 1)
        return;
      var textWindow = new TextWindow(this.selection.range);
      var editorWindow = new EditorWindow();
      editorWindow.appendChild(textWindow);
      editorWindow.realize();
      current.destroy();
    });

  Editor.bindKey(TextWindow, 'Ctrl+Shift+C',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(/** @type{!TextSelection} */(this.selection),
        function(selection) {
          selection.range.capitalize();
        });
    });

  Editor.bindKey(TextWindow, 'Ctrl+Shift+D',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(/** @type{!TextSelection}*/(this.selection),
        function(selection) {
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
      .then((x) => {
        const offset = isWhole ? 0 : this.selection.range.start;
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

  Editor.bindKey(TextWindow, 'Ctrl+Shift+U',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(/** @type{!TextSelection} */(this.selection),
        function(selection) {
          selection.range.toUpperCase();
        });
    });

  Editor.bindKey(TextWindow, 'Ctrl+V', pasteFromClipboardCommand);

  Editor.bindKey(TextWindow, 'Ctrl+W',
    /**
     * @this {!TextWindow}
     */
    function(arg) {
      var document = this.selection.document;
      document.close();
    });

  Editor.bindKey(TextWindow, 'Ctrl+X', cutToClipboardCommand);

  Editor.bindKey(TextWindow, 'Ctrl+Y',
    /**
     * @this {!TextWindow}
     */
    function() {
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

  Editor.bindKey(TextWindow, 'Ctrl+Z',
    /**
     * @this {!TextWindow}
     */
    function() {
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

  Editor.bindKey(TextWindow, 'Delete',
    /**
     * Forward delete character
     * @param {number=} opt_count
     * @this {!TextWindow}
     */
    function(opt_count) {
      var count = arguments.length >= 1 ? opt_count : 1;
      this.selection.range.delete(Unit.CHARACTER, count);
    });

  Editor.bindKey(TextWindow, 'End', function() {
    this.selection.endKey(Unit.WINDOW_LINE);
  }, 'move to end of window line\n' +
     'Move active position of selection to end of window line.');

  Editor.bindKey(TextWindow, 'Enter',
    /**
     * @param {number=} opt_count
     * @this {!TextWindow}
     */
    function(opt_count) {
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
  // TODO(eval1749): We should get |Indent|/|Outend| tab width from another place.
  var TAB_WIDTH = 4;

  Editor.bindKey(TextWindow, 'Shift+Tab',
    /** @param {number=} opt_count */
    function(opt_count) {
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

  Editor.bindKey(TextWindow, 'Tab',
    /** @param {number=} opt_count */
    function(opt_count) {
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
