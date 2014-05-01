// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.CppLexer = (function(keywords) {
  /** @enum{number} */
  var State = {
    ZERO: 0, // State.ZERO must be zero.

    BLOCK_COMMENT: 100,
    BLOCK_COMMENT_ASTERISK: 101,
    BLOCK_COMMENT_END: 102,
    BLOCK_COMMENT_START: 103,
    COLON: 200,
    COLON_COLON: 201,
    LINE_COMMENT: 300,
    LINE_COMMENT_ESCAPE: 301,
    LINE_COMMENT_START: 302,
    NEWLINE: 400,
    NUMBER_SIGN: 401,
    OPERATOR: 500,
    SOLIDUS: 600,
    SPACE: 700,
    STRING1: 800,
    STRING1_ESCAPE: 801,
    STRING2: 802,
    STRING2_ESCAPE: 803,
    WORD: 900
  };

  /** @const @type {!Map.<State, string>} */
  var StateToSyntax = new Map();
  StateToSyntax.set(State.ZERO, 'normal');
  StateToSyntax.set(State.BLOCK_COMMENT, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_ASTERISK, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_END, 'comment');
  StateToSyntax.set(State.BLOCK_COMMENT_START, 'comment');
  StateToSyntax.set(State.COLON, 'operators');
  StateToSyntax.set(State.COLON_COLON, 'operators');
  StateToSyntax.set(State.LINE_COMMENT, 'comment');
  StateToSyntax.set(State.LINE_COMMENT_ESCAPE, 'comment');
  StateToSyntax.set(State.LINE_COMMENT_START, 'comment');
  StateToSyntax.set(State.NEWLINE, 'normal');
  StateToSyntax.set(State.NUMBER_SIGN, 'operators');
  StateToSyntax.set(State.OPERATOR, 'operators');
  StateToSyntax.set(State.SOLIDUS, 'operators');
  StateToSyntax.set(State.SPACE, 'normal');
  StateToSyntax.set(State.STRING1, 'string_literal');
  StateToSyntax.set(State.STRING1_ESCAPE, 'string_literal');
  StateToSyntax.set(State.STRING2, 'string_literal');
  StateToSyntax.set(State.STRING2_ESCAPE, 'string_literal');
  StateToSyntax.set(State.WORD, 'identifier');

  Object.keys(State).forEach(function(key) {
    if (!StateToSyntax.has(State[key]))
      throw new Error('StateToSyntax must have ' + key);
  });

  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function CppLexer(document) {
    Lexer.call(this, keywords, document);
  }

  /**
   * @this {!CppLexer}
   * @param {number} maxCount
   */
  function doColor(maxCount) {
    if (!this.range)
      throw new Error("Can't use disconnected lexer.");

    this.adjustScanOffset();
    var range = this.range;
    var document = range.document;
    var maxOffset = document.length;
    this.count = maxCount;
    while (this.scanOffset < maxOffset) {
      var token = nextToken(this);
      if (!token)
        break;
      if (token.start == token.end) {
        // We are at end of document or scan more than |maxCount|.
        continue;
      }
      range.collapseTo(token.start);
      range.end = token.end;
      if (this.debug_ > 4)
        console.log('CppLexer', token, range);
      var syntax = StateToSyntax.get(token.state) || 'normal';
      if (syntax == 'identifier') {
        var word = extractWord(this, range, token);
        console.log('doColor', '"' + word + '"');
        if (this.keywords.has(word))
          syntax = 'keyword';
      }
      range.setSyntax(syntax);
    }

    return this.count;
  }

  /**
   * @this {!CppLexer}
   * @param {!Lexer.Token} token
   */
  function didShrinkLastToken(token) {
    console.log('didShrinkLastToken', token);
    if (token.state == State.BLOCK_COMMENT_END) {
      token.state = State.BLOCK_COMMENT_ASTERISK;
      return;
    }
    if (token.state == State.BLOCK_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
    if (token.state == State.COLON_COLON) {
      token.state = State.COLON;
      return;
    }
    if (token.state == State.LINE_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
    if (token.state == State.LINE_COMMENT_START) {
      token.state = State.SOLIDUS;
      return;
    }
  }

  /**
   * @param {!CppLexer} lexer
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}
   *
   * Extract CPP directive as '#" + word or namespace qualify identifier.
   */
  function extractWord(lexer, range, token) {
    var word = range.text;
    var it = lexer.tokens.find(token);
    if (!it)
      console.log('extractWord', token);
    do {
      it = it.previous();
    } while (it && it.data.state == State.SPACE);

    if (!it)
      return word;

    if (it.data.state == State.NUMBER_SIGN) {
      range.start = it.data.start;
      word = '#' + word;
      return word;
    }

    while (it && it.data.state == State.COLON_COLON) {
      range.start = it.data.start;
      word = '::' + word;
      do {
        it = it.previous();
        if (!it)
          return word;
      } while (it.data.state == State.SPACE);
      if (it.data.state != State.WORD)
        return word;
      var namespace = it.data;
      range.start = namespace.start;
      word = range.document.slice(namespace.start, namespace.end) + word;
      it = it.previous();
    }
    return word;
  }

  /**
   * @param {!Lexer.Token} token
   * @return {boolean}
   */
  function isBlockComment(token) {
    return token.state >= State.BLOCK_COMMENT &&
           token.state <= State.BLOCK_COMMENT_START;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isOperator(charCode) {
    return !isWhitespace(charCode) && !isWordRest(charCode);
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWhitespace(charCode) {
    return charCode == Unicode.LF || charCode == Unicode.SPACE ||
           charCode == Unicode.TAB;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordFirst(charCode) {
    if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
        charCode <= Unicode.LATIN_CAPITAL_LETTER_Z) {
      return true;
    }

    if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
        charCode <= Unicode.LATIN_SMALL_LETTER_Z) {
      return true;
    }

    return charCode == Unicode.LOW_LINE;
  }

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordRest(charCode) {
    if (isWordFirst(charCode))
      return true;
    return charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE;
  }

  /**
   * @param {!CppLexer} lexer
   * @return {?Lexer.Token}
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
      switch (lexer.state) {
        case State.BLOCK_COMMENT:
          if (charCode == Unicode.ASTERISK) {
            var token = lexer.finishToken();
            lexer.startToken(State.BLOCK_COMMENT_ASTERISK);
            ++lexer.scanOffset;
            return token;
          }
          ++lexer.scanOffset;
          break;

        case State.BLOCK_COMMENT_ASTERISK:
          if (charCode == Unicode.ASTERISK) {
            ++lexer.scanOffset;
            break;
          }

          if (charCode == Unicode.SOLIDUS)
            return lexer.finishTokenAs(State.BLOCK_COMMENT_END);

          {
            var token = lexer.finishToken();
            lexer.startToken(State.BLOCK_COMMENT);
            ++lexer.scanOffset;
            return token;
          }

        case State.BLOCK_COMMENT_START:
          lexer.startToken(State.BLOCK_COMMENT);
          break;

        case State.COLON:
          if (charCode == Unicode.COLON)
            return lexer.finishTokenAs(State.COLON_COLON);
          return lexer.finishToken();

        case State.LINE_COMMENT:
          if (charCode == Unicode.LF)
            return lexer.finishToken();

          if (charCode == Unicode.REVERSE_SOLIDUS) {
            var token = lexer.finishToken();
            lexer.startToken(State.LINE_COMMENT_ESCAPE);
            ++lexer.scanOffset;
            return token;
          }
          ++lexer.scanOffset;
          break;

        case State.LINE_COMMENT_ESCAPE: {
          ++lexer.scanOffset;
          var token = lexer.finishToken();
          lexer.startToken(State.LINE_COMMENT_ESCAPE);
          return token;
        }

        case State.NEWLINE:
          if (!isWhitespace(charCode))
            return lexer.finishToken();
          ++lexer.scanOffset;
          break;

        case State.OPERATOR:
          if (!isOperator(charCode))
            return lexer.finishToken();
          ++lexer.scanOffset;
          break;

        case State.SOLIDUS:
          if (charCode == Unicode.ASTERISK){
            var token = lexer.finishTokenAs(State.BLOCK_COMMENT_START);
            lexer.startToken(State.BLOCK_COMMENT);
            return token;
          }

          if (charCode == Unicode.SOLIDUS) {
            var token = lexer.finishTokenAs(State.LINE_COMMENT_START);
            lexer.startToken(State.LINE_COMMENT);
            return token;
          }
          ++lexer.scanOffset;
          break;

        case State.SPACE:
          if (charCode != Unicode.SPACE && charCode != Unicode.TAB)
            return lexer.finishToken();
          ++lexer.scanOffset;
          break;

        case State.STRING1:
          if (charCode == Unicode.QUOTATION_MARK) {
            ++lexer.scanOffset;
            return lexer.finishToken();
          }
          if (charCode == Unicode.REVERSE_SOLIDUS) {
            var token = lexer.finishToken();
            lexer.startToken(State.STRING1_ESCAPE);
            ++lexer.scanOffset;
            return token;
          }
          ++lexer.scanOffset;
          break;
        case State.STRING1_ESCAPE: {
          ++lexer.scanOffset;
          var token = lexer.finishToken();
          lexer.startToken(State.STRING1);
          return token;
        }

        case State.STRING2:
          if (charCode == Unicode.QUOTATION_MARK) {
            ++lexer.scanOffset;
            return lexer.finishToken();
          }
          if (charCode == Unicode.REVERSE_SOLIDUS) {
            var token = lexer.finishToken();
            lexer.startToken(State.STRING2_ESCAPE);

            ++lexer.scanOffset;
            return token;
          }
          ++lexer.scanOffset;
          break;
        case State.STRING2_ESCAPE: {
          ++lexer.scanOffset;
          var token = lexer.finishToken();
          lexer.startToken(State.STRING2);
          return token;
        }

        case State.WORD:
          if (!isWordRest(charCode))
            return lexer.finishToken();
          ++lexer.scanOffset;
          break;

        case State.ZERO:
          switch (charCode) {
            case Unicode.APOSTROPHE:
              lexer.startToken(State.STRING1);
              ++lexer.scanOffset;
              break;
            case Unicode.COLON:
              lexer.startToken(State.COLON);
              ++lexer.scanOffset;
              break;
            case Unicode.LF:
              lexer.startToken(State.NEWLINE);
              ++lexer.scanOffset;
              break;
            case Unicode.NUMBER_SIGN:
              if (!lexer.lastToken || lexer.lastToken.state == State.NEWLINE) {
                lexer.startToken(State.NUMBER_SIGN);
                ++lexer.scanOffset;
                return lexer.finishToken();
              }
              lexer.startToken(State.OPERATOR);
              ++lexer.scanOffset;
              break;
            case Unicode.QUOTATION_MARK:
              lexer.startToken(State.STRING2);
              ++lexer.scanOffset;
              break;
            case Unicode.SOLIDUS:
              lexer.startToken(State.SOLIDUS);
              ++lexer.scanOffset;
              break;
            case Unicode.SPACE:
            case Unicode.TAB:
              lexer.startToken(State.SPACE);
              ++lexer.scanOffset;
              break;
            default:
              if (isWordRest(charCode))
                lexer.startToken(State.WORD);
              else
                lexer.startToken(State.OPERATOR);
              ++lexer.scanOffset;
              break;
          }
          break;
        default:
          console.log(lexer);
          throw new Error('Invalid state ' + lexer.state);
      }
    }
    if (lexer.lastToken)
      lexer.lastToken.end = lexer.scanOffset;
    return lexer.lastToken;
  }

  CppLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: CppLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    doColor: {value: doColor}
  });

  return CppLexer;
})([
  // C++ keywords
  'auto', 'bool', 'break', 'case', 'catch', 'char', 'class', 'const',
  'constexpr', 'const_cast', 'continue', 'decltype', 'default', 'delete',
  'do', 'double', 'else', 'enum', 'extern', 'false', 'final', 'finally',
  'float', 'for', 'friend', 'goto', 'if', 'inline', 'int', 'long',
  'mutable', 'namespace', 'new', 'nullptr', 'operator', 'override', 'private',
  'protected', 'public', 'register', 'reinterpret_cast', 'return', 'short',
  'signed', 'sizeof', 'static', 'static_assert', 'static_cast', 'struct',
  'switch', 'template', 'try', 'typedef', 'typeid', 'typename', 'union',
  'unsigned', 'using', 'virtual', 'void', 'volatile', 'while',

  // C Pre-Processor
  '#define',
  '#else',
  '#elif',
  '#endif',
  '#error',
  '#if',
  '#ifdef',
  '#ifndef',
  '#import', // MSC
  '#include',
  '#line',
  '#pragma',
  '#undef',
  '#using', // MSC

  // ANSI-C Predefined Macros
  '__DATE__',
  '__FILE__',
  '__LINE__',
  '__STDC__',
  '__TIME__',
  '__TIMESTAMP__',

  // MSC Predefined Macros
  '_ATL_VER',
  '_CHAR_UNSIGNED',
  '__CLR_VER',
  '__cplusplus_cli',
  '__COUNTER__',
  '__cplusplus',
  '__CPPLIB_VER',
  '__CPPRTTI',
  '__CPPUNWIND',
  '__DEBUG',
  '_DL',
  '__FUNCDNAME__',
  '__FUNCSIG__',
  '__FUNCTION__',
  '_INTEGRAL_MAX_BITS',
  '_M_ALPHA',
  '_M_CEE',
  '_M_CEE_PURE',
  '_M_CEE_SAFE',
  '_M_IX86',
  '_MIA64',
  '_M_IX86_FP',
  '_M_MPPC',
  '_M_MRX000',
  '_M_PPC',
  '_M_X64',
  '_MANAGED',
  '_MFC_VER',
  '_MSC_EXTENSIONS',
  '_MSC_VER',
  '_MSVC_RUNTIME_CHECKES',
  '_MT',
  '_NATIVE_WCHAR_T_DEFINED',
  '_OPENMP',
  '_VC_NODEFAULTLIB',
  '_WCHAR_T_DEFINED',
  '_WIN32',
  '_WIN64',
  '_Wp64',

  // <stdint.h>
  'int16_t', 'int32_t', 'int64_t', 'uint8_t', 'uint16_t', 'uint32_t',
  'uint64_t', 'uint8_t',

  // Standard C Library
  'NULL', 'div_t', 'ldiv_t', 'ptrdiff_t', 'size_t', 'va_arg', 'va_end',
  'va_list', 'va_start', 'wchar_t',

  // C++ library
  'std::find',
  'std::iterator', 'std::list', 'std::max', 'std::min', 'std::move',
  'std::string', 'std::unique_ptr', 'std::unordered_map', 'std::unordered_set',
  'std::vector',

  // Chromium
  'arraysize', 'base::Bind', 'base::Time', 'base::char16', 'base::string16',
  'scoped_refptr',

  // Microsoft C++
  '__abstract', '__alignof', '__asm', '__assume', '__based', '__box',
  '__cdecl', '__declspec', 'deprecated', 'dllexport', 'dllimport',

  '__event', '__except', '__fastcall', '__finally', '__forceinline',
  '__inline', '__int8', '__int16', '__int32', '__int64', '__interface',
  '__m64', '__m128', '__m128d', '__m128i', '__pragma', '__raise', '__sealed',
  'selectany', '__stdcall', '__super', '__unaligned', '__unhook', '__uuidof',
  '__value', '__w64',

  // WIN32
  'BOOL', 'DWORD', 'HANDLE', 'HIWORD', 'HKEY', 'INVALID_HANDLE_VALUE',
  'LOWROD', 'WORD',

  // v8
  'v8::ArrayBuffer',
  'v8::Context',
  'v8::EscapableHandleScope', 'v8::Eternal',
  'v8::FunctionCallback', 'v8::FunctionTemplate',
  'v8::Handle', 'v8::HandleScope',
  'v8::Isolate',
  'v8::Local',

  'v8::Object', 'v8::ObjectTemplate',
  'v8::Persistent',
  'v8::Promise',
  'v8::Script',
  'v8::Undefined', 'v8::UniquePersistent',
  'v8::Value',

  // Vogue
  'gfx::Point', 'gfx::Rect', 'gfx::Size',
  'gfx::PointF', 'gfx::RectF', 'gfx::SizeF',
]);
