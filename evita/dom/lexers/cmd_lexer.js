// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.CmdLexer = (function() {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    CIRCUMFLEX_ACCENT: 1,
    COLON: 2,
    COMMENT: 3,
    LABEL: 4,
    NEWLINE: 5,
    STRING: 6,
    WORD: 7
  };

  /** @const @type {Set.<string>} */
  var KEYWORDS = (function() {
    var set = new Set();
    [
      'assoc', 'attrib', 'break', 'bcdedit', 'cacls', 'call', 'cd', 'chcp',
      'chdir', 'chkdsk', 'chkntfs', 'cls', 'cmd', 'color', 'comp', 'compact',
      'convert', 'copy', 'date', 'del', 'dir', 'diskcomp', 'diskcopy',
      'diskpart', 'doskey', 'driverquery', 'echo', 'endlocal', 'erase', 'exit',
      'fc', 'find', 'findstr', 'for', 'format', 'fsutil', 'ftype', 'goto',
      'gpresult', 'graftabl', 'help', 'icacls', 'if', 'label', 'md', 'mkdir',
      'mklink', 'mode', 'more', 'move', 'openfiles', 'path', 'pause', 'popd',
      'print', 'prompt', 'pushd', 'rd', 'recover', 'rem', 'ren', 'rename',
      'replace', 'rmdir', 'robocopy', 'set', 'setlocal', 'sc', 'schtasks',
      'shift', 'shutdown', 'sort', 'start', 'subst', 'systeminfo', 'tasklist',
      'taskkill', 'time', 'title', 'tree', 'type', 'ver', 'verify', 'vol',
      'xcopy', 'wmic',
    ].forEach(function(keyword) {
      set.add(keyword);
    });
    return set;
  })();

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function CmdLexer(document) {
    Lexer.call(this, document);
  }

  /**
   * @this {!CmdLexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset(maxCount);
    var range = this.range;
    var document = range.document;
    var maxOffset = document.length;
    while (this.scanOffset < maxOffset) {
      var syntax = nextToken(this);
      if (!syntax)
        break;
      if (range.collapsed)
        continue;
      if (this.debug > 0)
        console.log('CmdLexer', syntax, range);
      if (syntax == 'cmd_newline')
        this.numberOfWords_ = 0;
      else
        ++this.numberOfWords_;
      if (syntax == 'cmd_command') {
        var command = range.text;
       if (KEYWORDS.has(command.toLowerCase()))
          syntax = 'keyword';
      }
      range.setSyntax(syntax);
    }

    return this.count;
  }

  /**
   * @param {!CmdLexer} lexer
   * @return {?string}
   */
  function nextToken(lexer) {
    if (!lexer.count)
      return null;
    var document = lexer.range.document;
    var maxOffset = document.length;
    if (lexer.scanOffset == maxOffset)
      return null;
    while (lexer.count) {
      --lexer.count;
      if (lexer.scanOffset == maxOffset)
        break;
      var charCode = document.charCodeAt_(lexer.scanOffset);
      ++lexer.scanOffset;
      switch (lexer.state) {
        case State.CIRCUMFLEX_ACCENT:
          lexer.state = State.ZERO;
          break;
        case State.COLON:
          switch (charCode) {
            case Unicode.LF:
              --lexer.scanOffset;
              return lexer.finishToken();
            case Unicode.TAB:
            case Unicode.SPACE:
              --lexer.scanOffset;
              lexer.startToken(State.COMMENT, 'comment');
              break;
            default:
              --lexer.scanOffset;
              lexer.startToken(State.LABEL, 'cmd_label');
              break;
          }
          break;
        case State.COMMENT:
          if (charCode != Unicode.LF)
            break;
          --lexer.scanOffset;
          return lexer.finishToken();
        case State.LABEL:
          if (charCode != Unicode.LF)
            break;
          --lexer.scanOffset;
          return lexer.finishToken();
        case State.STRING:
          if (charCode == Unicode.QUOTATION_MARK)
            return lexer.finishToken();
          if (charCode == Unicode.LF) {
            --lexer.scanOffset;
            return lexer.finishToken();
          }
          break;

        case State.WORD:
          switch (charCode) {
            case Unicode.CIRCUMFLEX_ACCENT:
            case Unicode.LF:
            case Unicode.QUOTATION_MARK:
            case Unicode.SPACE:
            case Unicode.TAB:
              --lexer.scanOffset;
              return lexer.finishToken();
           }
           break;
        case State.ZERO:
          switch (charCode) {
            case Unicode.CIRCUMFLEX_ACCENT:
              lexer.start = State.CIRCUMFLEX_ACCENT;
              break;
            case Unicode.COLON:
              lexer.startToken(State.COLON, 'cmd_colon');
              break;
            case Unicode.LF:
              lexer.startToken(State.NEWLINE, 'cmd_newline');
              return lexer.finishToken();
            case Unicode.SPACE:
            case Unicode.TAB:
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING, 'string_literal');
              break;
            default:
              lexer.startToken(State.WORD, 'cmd_command');
              break;
          }
          break;
        default:
          throw new Error('Invalid state ' + lexer.state);
      }
    }
    lexer.range.end = lexer.scanOffset;
    return lexer.syntax;
  }

  CmdLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: CmdLexer},
    doColor: {value: doColor},
    numberOfWords_: {value: 0, writable: true}
  });

  return CmdLexer;
})();
