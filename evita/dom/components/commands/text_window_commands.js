// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @type {*} */
var $0;

goog.scope(function() {

/** @const @type {!Array<number>} */
const ZOOM_STEPS = [
  20,  30,  40,  50,  60,  70,  80,  90,  100, 110,
  130, 150, 170, 200, 250, 300, 350, 400, 450, 500,
];

/** @param {!TextRange} range */
function copyToClipboard(range) {
  /** @const @type {!DataTransferItemList} */
  const items = DataTransfer.clipboard.items;
  items.clear();
  items.add(range.text.replace(/\n/g, '\r\n'), 'text/plain');
}

/**
 * @param {number} charCode
 * @param {number} base
 * @return {number}
 */
function fromDigitChar(charCode, base) {
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
}

/**
 * @param {number} charCode
 */
function makeTypeCharCommand(charCode) {
  return function(count = 1) { typeChar.call(this, charCode, count); };
}

/**
 * Paste from clipboard
 * @this {!TextWindow}
 */
function pasteFromClipboardCommand() {
  /** @const @type {!DataTransferItemList} */
  const items = DataTransfer.clipboard.items;
  if (!items.length)
    return;
  /** @const @type {!DataTransferItem} */
  const item = items.get(0);
  if (item.kind !== 'string')
    return;
  const range = this.selection.range;
  range.text = item.getAsString().replace(/\r\n/g, '\n');
  range.collapseTo(range.end);
}

/**
 * @param {string} text
 * @param {number} base
 */
function safeParseInt(text, base) {
  /** @type {number} */
  let value = 0;
  for (/** @type {number} */ let k = 0; k < text.length; ++k) {
    const digit = fromDigitChar(text.charCodeAt(k), base);
    if (isNaN(digit))
      return NaN;
    value *= base;
    value += digit;
  }
  return value;
}

/**
 * Type character
 * @this {!TextWindow}
 * @param {number} charCode
 * @param {number=} count
 */
function typeChar(charCode, count = 1) {
  const range = this.selection.range;
  range.text = String.fromCharCode(charCode).repeat(count);
  range.collapseTo(range.end);
}

// Install |TypeChar| commands.
for (let charCode = 0x20; charCode < 0x7F; ++charCode) {
  Editor.bindKey(
      TextWindow, String.fromCharCode(charCode), makeTypeCharCommand(charCode),
      'type character ' + String.fromCharCode(charCode));
}

/**
 * Backward delete character
 * @param {number=} count
 * @this {!TextWindow}
 */
function backspaceCommand(count = 1) {
  this.selection.range.delete(Unit.CHARACTER, -count);
}
Editor.bindKey(TextWindow, 'Backspace', backspaceCommand);

/**
 * Zoom-In
 * @this {!TextWindow}
 */
function zoomInCommand() {
  /** @const @type {number} */
  const currentZoom = Math.round(this.zoom * 100);
  /** @const @type {number} */
  const index =
      ZOOM_STEPS.findIndex(function(zoom) { return zoom > currentZoom; });
  if (index < 0)
    return;
  this.zoom = ZOOM_STEPS[index] / 100;
}
Editor.bindKey(TextWindow, 'Ctrl++', zoomInCommand, 'Zoom In');

Editor.bindKey(
    TextWindow, 'Ctrl+,',
    /**
     * Reconvert text using IME.
     * @this {!TextWindow}
     */
    function() {
      /** @const @type {!TextRange} */
      const range = this.selection.range;
      this.reconvert_(range.text);
      range.text = '';
    },
    'Reconvert');

/**
 * Zoom-Out
 * @this {!TextWindow}
 */
function zoomOutCommand() {
  /** @const @type {number} */
  const currentZoom = Math.round(this.zoom * 100);
  /** @const @type {number} */
  const index =
      ZOOM_STEPS.findIndex(function(zoom) { return zoom >= currentZoom; });
  if (index < 1)
    return;
  this.zoom = ZOOM_STEPS[index - 1] / 100;
}
Editor.bindKey(TextWindow, 'Ctrl+-', zoomOutCommand, 'Zoom out');

/**
 * Exchange Unicode character code point and character
 * @this {!TextWindow}
 * @param {number=} base
 */
function codePointCommand(base = 16) {
  if (base < 2 || base > 36)
    return;
  // TODO(eval1749): We should use |TextDocument.prototype.replace()|.
  /** @const @type {!TextRange} */
  const range = new TextRange(this.selection.range);
  if (range.start === range.end) {
    range.moveStart(Unit.WORD, -1);
    if (range.length > 4)
      range.start = range.end - 4;
  } else if (range.length > 4) {
    return;
  }
  /** @const @type {string} */
  const text = range.text;
  if (!text.length)
    return;
  /** @const @type {number} */
  const charCode = safeParseInt(text, base);
  if (isNaN(charCode)) {
    range.start = range.end - 1;
    range.text =
        ('000' + text.charCodeAt(text.length - 1).toString(16)).substr(-4);
  } else {
    range.text = String.fromCharCode(charCode);
  }
  this.selection.range.collapseTo(range.end);
}
Editor.bindKey(
    TextWindow, 'Ctrl+.', codePointCommand,
    'Exchange Unicode code point and character\n');

/** @this {!TextWindow} */
function resetZoomCommand() {
  this.zoom = 1.0;
}
Editor.bindKey(TextWindow, 'Ctrl+0', resetZoomCommand);

/**
 * Select all contents
 * @this {!TextWindow}
 */
function selectAll() {
  /** @const @type {!TextRange} */
  const range = this.selection.range;
  range.start = 0;
  range.end = range.document.length;
  this.selection.startIsActive = false;
}
Editor.bindKey(TextWindow, 'Ctrl+A', selectAll);

/**
 * Backward delete word
 * @param {number=} count
 * @this {!TextWindow}
 */
function deleteWordBackwardCommand(count = 1) {
  this.selection.range.delete(Unit.WORD, -count);
}
Editor.bindKey(TextWindow, 'Ctrl+Backspace', deleteWordBackwardCommand);

/** @this {!TextWindow} window */
function copyToClipboardCommand() {
  copyToClipboard(this.selection.range);
}
Editor.bindKey(TextWindow, 'Ctrl+C', copyToClipboardCommand);

/**
 * Forward delete word
 * @param {number=} count
 * @this {!TextWindow}
 */
function deleteWordForwardCommand(count = 1) {
  this.selection.range.delete(Unit.WORD, count);
}
Editor.bindKey(TextWindow, 'Ctrl+Delete', deleteWordForwardCommand);

/** @this {!TextWindow} */
function moveToEndOfDocumentCommand() {
  this.selection.endKey(Unit.DOCUMENT);
}
Editor.bindKey(
    TextWindow, 'Ctrl+End', moveToEndOfDocumentCommand,
    'move to end of document\n' +
        'Move active position of selection to end of document.');

/**
 * @param {TextSelection} selection
 * @param {function(TextSelection)} changer
 */
function changeCase(selection, changer) {
  if (selection.range.collapsed) {
    selection.range.startOf(Unit.WORD, Alter.EXTEND);
    selection.range.endOf(Unit.WORD, Alter.EXTEND);
  }
  changer(selection);
}

// TODO(eval1749): We should display dialog box to prompt enter line number
// and
// list of functions.
/**
 * @this {!TextWindow}
 * @param {number|undefined} arg
 */
function gotoLineCommand(arg) {
  if (!arg)
    return;
  this.selection.range.startOf(Unit.DOCUMENT);
  this.selection.range.move(Unit.LINE, arg - 1);
}
Editor.bindKey(TextWindow, 'Ctrl+G', gotoLineCommand);

/** @this {!TextWindow} */
function startOfDocumentCommand() {
  this.selection.homeKey(Unit.DOCUMENT);
}
Editor.bindKey(
    TextWindow, 'Ctrl+Home', startOfDocumentCommand,
    'move to home of document\n' +
        'Move active position of selection to home of document.');

/**
* Make selection visible.
* @this {!TextWindow}
*/
function makeSelectionVisibleCommand() {
  this.makeSelectionVisible();
}
Editor.bindKey(TextWindow, 'Ctrl+L', makeSelectionVisibleCommand);

/**
 * Reload document
 * @this {!TextWindow}
 */
function reloadDocumentCommand(arg) {
  const document = this.document;
  if (!document.needSave())
    return;
  /** @const @type {!TextSelection} */
  const selection = this.selection;
  /** @const @type {number} */
  const offset = selection.range.start;
  Editor
      .messageBox(
          this,
          Editor.localizeText(Strings.IDS_ASK_RELOAD, {name: document.name}),
          MessageBox.ICONQUESTION | MessageBox.YESNO)
      .then(function(responseCode) {
        if (responseCode !== DialogItemId.YES)
          return;
        document.load().then(function() {
          selection.range.collapseTo(offset);
        });
      });
}
Editor.bindKey(TextWindow, 'Ctrl+R', reloadDocumentCommand);

/**
 * Save file
 * @this {!TextWindow}
 */
function saveDocumentCommand(arg) {
  /** @const @type {!TextDocument} */
  const document = this.selection.document;
  if (!arg && document.fileName !== '') {
    document.save(document.fileName);
    return;
  }

  Editor.getFileNameForSave(this, document.fileName).then(function(fileName) {
    document.save(fileName);
  });
}
Editor.bindKey(TextWindow, 'Ctrl+S', saveDocumentCommand);

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+3',
    /**
     * Show document in new editor window.
     * @this {!TextWindow}
     */
    function() {
      /** @const @type {!TextWindow} */
      const textWindow = new TextWindow(this.selection.range);
      /** @const @type {!EditorWindow} */
      const editorWindow = new EditorWindow();
      editorWindow.appendChild(textWindow);
      editorWindow.realize();
    });

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+4',
    /**
     * Show document in new editor window and close current editor window.
     * @this {!TextWindow}
     */
    function() {
      /** @const @type {?Window} */
      const current = this.selection.window.parent;
      // When editor window has only one tab, we don't ignore this command,
      // since result of this command isn't useful.
      if (!current || current.children.length === 1)
        return;
      const textWindow = new TextWindow(this.selection.range);
      const editorWindow = new EditorWindow();
      editorWindow.appendChild(textWindow);
      editorWindow.realize();
      current.destroy();
    });

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+C',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(
          /** @type{!TextSelection} */ (this.selection),
          function(selection) { selection.range.capitalize(); });
    });

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+D',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(
          /** @type{!TextSelection}*/ (this.selection),
          function(selection) { selection.range.toLowerCase(); });
    });

/**
 * Evaluate script in selection selection
 * @this {!TextWindow}
 */
function evalSelectionCommand() {
  /** @const @type {!TextSelection} */
  const selection = this.selection;
  /** @const @type {boolean} */
  const isWhole = selection.range.start === selection.range.end;
  /** @const @type {string} */
  const scriptText = isWhole ?
      (new TextRange(this.document, 0, this.document.length)).text :
      selection.range.text;
  $0 = this;
  const result = Editor.runScript(scriptText, this.document.name);
  if (!result.exception) {
    if (result.value !== undefined)
      console.log(result.value);
    return;
  }
  Editor
      .messageBox(
          this, result.stackTraceString, MessageBox.ICONERROR,
          'Evaluate Selection Command')
      .then((x) => {
        const offset = isWhole ? 0 : this.selection.range.start;
        selection.range.collapseTo(offset + result.start)
            .moveEnd(Unit.CHARACTER, result.end - result.start);
      });
}
Editor.bindKey(
    TextWindow, 'Ctrl+Shift+E', evalSelectionCommand,
    'Evaluate script in selection');
Editor.bindKey(
    TextWindow, 'F5', evalSelectionCommand, 'Evaluate script in selection');

Editor.bindKey(TextWindow, 'Ctrl+Shift+Delete', copyToClipboardCommand);

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+End',
    function() { this.selection.endKey(Unit.DOCUMENT, Alter.EXTEND); },
    'extend to end of document\n' +
        'Move active position of selection to end of document.');

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+Home',
    function() { this.selection.homeKey(Unit.DOCUMENT, Alter.EXTEND); },
    'exthome to home of document\n' +
        'Move active position of selection to home of document.');

Editor.bindKey(
    TextWindow, 'Ctrl+Shift+U',
    /**
     * @this {!TextWindow}
     */
    function() {
      changeCase(
          /** @type{!TextSelection} */ (this.selection),
          function(selection) { selection.range.toUpperCase(); });
    });

Editor.bindKey(TextWindow, 'Ctrl+V', pasteFromClipboardCommand);

/**
 * @this {!TextWindow}
 */
function closeDocumentCommand(arg) {
  this.selection.document.close();
}
Editor.bindKey(TextWindow, 'Ctrl+W', closeDocumentCommand);

/** @this {!TextWindow} window */
function cutToClipboardCommand() {
  /** @const @type {!TextRange} */
  const range = this.selection.range;
  copyToClipboard(range);
  range.text = '';
}
Editor.bindKey(TextWindow, 'Ctrl+X', cutToClipboardCommand);

/**
 * @this {!TextWindow}
 */
function redoCommand() {
  /** @const @type {!TextSelection} */
  const selection = this.selection;
  /** @const @type {number} */
  const offset = selection.document.redo(selection.focusOffset);
  if (offset < 0) {
    Editor.messageBox(
        this, Editor.localizeText(Strings.IDS_NO_MORE_REDO),
        MessageBox.ICONWARNING);
    return;
  }
  selection.range.collapseTo(offset);
}
Editor.bindKey(TextWindow, 'Ctrl+Y', redoCommand);

/** @this {!TextWindow} */
function undoCommand() {
  /** @const @type {!TextSelection} */
  const selection = this.selection;
  /** @const @type {number} */
  const offset = selection.document.undo(selection.focusOffset);
  if (offset < 0) {
    Editor.messageBox(
        this, Editor.localizeText(Strings.IDS_NO_MORE_UNDO),
        MessageBox.ICONWARNING);
    return;
  }
  selection.range.collapseTo(offset);
}
Editor.bindKey(TextWindow, 'Ctrl+Z', undoCommand);

/**
 * Forward delete character
 * @param {number=} count
 * @this {!TextWindow}
 */
function deleteCharacterForwadCommand(count = 1) {
  this.selection.range.delete(Unit.CHARACTER, count);
}
Editor.bindKey(TextWindow, 'Delete', deleteCharacterForwadCommand);

/** @this {!TextWindow} */
function moveToEndOfWindowLineCommand() {
  this.selection.endKey(Unit.WINDOW_LINE);
}
Editor.bindKey(
    TextWindow, 'End', moveToEndOfWindowLineCommand,
    'move to end of window line\n' +
        'Move active position of selection to end of window line.');

/**
 * @param {number=} count
 * @this {!TextWindow}
 */
function enterKeyCommand(count = 1) {
  if (count <= 0)
    return;
  /** @const @type {!TextSelection} */
  const selection = this.selection;
  /** @const @type {string} */
  const leadingWhitespaces = (new TextRange(selection.range))
                                 .startOf(Unit.LINE)
                                 .moveEndWhile(' \t')
                                 .text;
  selection.document.undoGroup('TypeEnter', function() {
    selection.range.moveStartWhile(' \t', Count.BACKWARD);
    selection.range.text = '\n'.repeat(/** @type {number} */ (count));
    selection.range.collapseTo(selection.range.end).moveEndWhile(' \t').text =
        leadingWhitespaces;
    selection.range.collapseTo(selection.range.end);
  });
}
Editor.bindKey(
    TextWindow, 'Enter', enterKeyCommand, 'type enter key\n' +
        'Insert newlines with leading whitespaces and remove trailing' +
        ' whitespaces.');

/** @this {!TextWindow} */
function startOfWindowLineCommand() {
  this.selection.homeKey(Unit.WINDOW_LINE);
}
Editor.bindKey(
    TextWindow, 'Home', startOfWindowLineCommand,
    'move to home of window line\n' +
        'Move active position of selection to home of window line.');

Editor.bindKey(TextWindow, 'Shift+Delete', cutToClipboardCommand);

Editor.bindKey(
    TextWindow, 'Shift+End',
    function() { this.selection.endKey(Unit.WINDOW_LINE, Alter.EXTEND); },
    'extend to end of window line\n' +
        'Move active position of selection to end of window line.');

/** @this {!TextWindow} */
function extendToStartOfWindowLineCommand() {
  this.selection.homeKey(Unit.WINDOW_LINE, Alter.EXTEND);
}

Editor.bindKey(
    TextWindow, 'Shift+Home', extendToStartOfWindowLineCommand,
    'extend to home of window line\n' +
        'Move active position of selection to home of window line.');

Editor.bindKey(TextWindow, 'Shift+Insert', pasteFromClipboardCommand);

// TODO(eval1749): We should get |Indent|/|Outdent| tab width from another
// place.
/** @const @type {number} */
const TAB_WIDTH = 4;

/**
 * @this {!TextWindow}
 * @param {number=} tabWidth*
 */
function outdentCommand(tabWidth = TAB_WIDTH) {
  /** @const @type {!TextRange} */
  const range = this.selection.range;
  if (range.collapsed) {
    // Move to previous tab stop
    range.startOf(Unit.LINE, Alter.EXTEND);
    range.collapseTo(
        Math.floor((range.start - range.end - 1) / tabWidth) * tabWidth +
        range.end);
    return;
  }
  // Expand range to contain whole lines
  range.startOf(Unit.LINE, Alter.EXTEND);
  // Remove leading whitespaces
  range.document.undoGroup('Outdent', function() {
    /** @const @type {!TextDocument} */
    const document = this;
    // TODO(eval1749): We should use |TextDocument.prototype.replace()|.
    /** @const @type {!TextRange} */
    const lineRange = new TextRange(range);
    lineRange.collapseTo(lineRange.start);
    while (lineRange.start < range.end) {
      lineRange.moveEndWhile(' ', tabWidth);
      if (lineRange.end < document.length &&
          lineRange.end - lineRange.start < tabWidth &&
          document.charCodeAt(lineRange.end) === 0x09) {
        lineRange.moveEnd(Unit.CHARACTER);
      }
      lineRange.text = '';
      lineRange.endOf(Unit.LINE).move(Unit.CHARACTER);
    }
    range.end = lineRange.start;
  });
}

Editor.bindKey(
    TextWindow, 'Shift+Tab', outdentCommand, 'outdent\n' +
        'Remove leading whitespaces from lines in selection.');

/**
 * @this {!TextWindow}
 * @param {number=} tabWidth
 */
function indentCommand(tabWidth = TAB_WIDTH) {
  /** @const @type {!TextRange} */
  const range = this.selection.range;
  if (range.start === range.end) {
    /** @const @type {number} */
    const current = range.start;
    range.startOf(Unit.LINE, Alter.EXTEND);
    /** @const @type {number} */
    const numSpaces = tabWidth - (current - range.start) % tabWidth;
    range.start = range.end;
    range.insertBefore(' '.repeat(numSpaces));
    return;
  }

  // Expand range to contain whole lines
  range.startOf(Unit.LINE, Alter.EXTEND);
  range.document.undoGroup('Indent', function() {
    // TODO(eval1749): We should use |TextDocument.prototype.replace()|.
    /** @const @type {!TextRange} */
    const lineRange = new TextRange(range);
    lineRange.collapseTo(lineRange.start);
    while (lineRange.start < range.end) {
      /** @const @type {string} */
      const spaces = ' '.repeat(/** @type{number} */ (tabWidth));
      lineRange.endOf(Unit.LINE, Alter.EXTEND);
      if (lineRange.start !== lineRange.end)
        lineRange.insertBefore(spaces);
      lineRange.move(Unit.CHARACTER);
    }
    range.end = lineRange.start;
  });
  // Make selection contains spaces inserted at first line.
  range.startOf(Unit.LINE, Alter.EXTEND);
  this.selection.startIsActive = false;
}

Editor.bindKey(
    TextWindow, 'Tab', indentCommand, 'indent\n' +
        'Caret: insert spaces until tab stop column.\n' +
        'TextRange: insert spaces all lines in range.');
});
