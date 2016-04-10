// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const CppTokenStateMachine = highlights.CppTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;
const TokenMap = highlights.base.TokenMap;

/** @const @type {!Set<string>} */
const staticCppKeywords = new Set();

class CppPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   * @param {!TokenMap} tokenMap
   */
  constructor(document, tokenMap) { super(document, tokenMap); }

  /**
   * @private
   * @param {!Token} token
   * @return {boolean}
   */
  isColonToken(token) {
    return token.syntax === 'operator' && token.end - token.start === 1 &&
        this.document.charCodeAt(token.start) == Unicode.COLON;
  }

  /**
   * @private
   * @param {!Token} token
   * @return {boolean}
   */
  isColonColonToken(token) {
    return token.syntax === 'operator' && token.end - token.start === 1 &&
        this.document.charCodeAt(token.start) == Unicode.COLON;
  }

  /**
   * @private
   * @param {!Token} token
   * @return {boolean}
   */
  isNumberSign(token) {
    return token.syntax === 'keyword' && token.end - token.start === 1 &&
        this.document.charCodeAt(token.start) == Unicode.NUMBER_SIGN;
  }

  /**
   * @private
   * @param {!Token} startToken
   * @param {!Token} token
   */
  paintQualifiedName(startToken, token) {
    console.assert(startToken !== token, startToken, token);
    const qualifiedName = this.document.slice(startToken.start, token.end);
    if (staticCppKeywords.has(qualifiedName))
      return this.document.setSyntax(startToken.start, token.end, 'keyword');
    // Repaint namespace prefix
    this.document.setSyntax(startToken.start, token.start, 'identifier');
    return this.paintSimpleName(token);
  }


  /**
   * @private
   * @param {!Token} token
   */
  paintSimpleName(token) {
    const name = this.textOf(token);
    const syntax = staticCppKeywords.has(name) ? 'keyword' : 'identifier';
    this.document.setSyntax(token.start, token.end, syntax);
  }

  /**
   * @override
   * @param {!Token} token
   * This function does following custom painting for C++ language:
   *  - label, name ends with ":".
   *  - C++ keywords
   *  - qualified name; namespace? "::" name. (one namespace prefix only)
   *
   */
  paint(token) {
    if (this.isColonToken(token)) {
      const previousToken = this.previousTokenOf(token);
      if (previousToken === null)
        return this.paintToken(token);
      if (previousToken.syntax == 'identifier') {
        // name + ':'
        this.document.setSyntax(
            previousToken.start, previousToken.end, 'label');
        return this.paintToken(token);
      }
      if (this.isColonToken(previousToken)) {
        // Attempt to get rid of label color for "name::".
        const maybeNameToken = this.previousTokenOf(previousToken);
        if (maybeNameToken === null)
          return this.paintToken(token);
        if (maybeNameToken.syntax !== 'identifier')
          return this.paintToken(token);
        // name + ':' + ':'
        this.paintToken(maybeNameToken);
        return this.paintToken(token);
      }
      return this.paintToken(token);
    }

    if (token.syntax !== 'identifier')
      return this.paintToken(token);

    const previousToken = this.previousTokenOf(token);
    if (previousToken === null)
      return this.paintSimpleName(token);

    if (this.isNumberSign(previousToken)) {
      // '#' + name
      return this.paintQualifiedName(previousToken, token);
    }

    if (this.isColonToken(previousToken)) {
      const colon2Token = this.previousTokenOf(previousToken);
      if (colon2Token === null) {
        // ^ ':' name
        return this.paintSimpleName(token);
      }
      const namespaceToken = this.previousTokenOf(colon2Token);
      if (namespaceToken === null || namespaceToken.syntax !== 'identifier') {
        // '::' + name
        return this.paintQualifiedName(colon2Token, token);
      }
      // name + '::' + name
      return this.paintQualifiedName(namespaceToken, token);
    }

    this.paintSimpleName(token);
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @param {!TokenMap} tokenMap
   * @return {!Painter}
   */
  static create(document, tokenMap) {
    return new CppPainter(document, tokenMap);
  }
}

class CppHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, CppPainter.create, new CppTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticCppKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticCppKeywords.add(word); }
}

Object.freeze(CppHighlighter);
Object.seal(CppHighlighter);

[
    // C++ keywords
    'alignas', 'alignof', 'and', 'and_eq', 'asm', 'auto', 'bitand', 'bitoir',
    'bool', 'break', 'case', 'catch', 'char', 'char16_t', 'char32_t', 'class',
    'const', 'constexpr', 'const_cast', 'continue', 'decltype', 'default',
    'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum', 'explicit',
    'export', 'extern', 'false', 'final', 'finally', 'float', 'for', 'friend',
    'goto', 'if', 'inline', 'int', 'long', 'mutable', 'namespace', 'new',
    'noexcept', 'not', 'not_eq', 'nullptr', 'operator', 'or', 'or_eq',
    'override', 'private', 'protected', 'public', 'register',
    'reinterpret_cast', 'return', 'short', 'signed', 'sizeof', 'static',
    'static_assert', 'static_cast', 'struct', 'switch', 'template', 'this',
    'thread_local', 'throw', 'true', 'try', 'typedef', 'typeid', 'typename',
    'union', 'unsigned', 'using', 'virtual', 'void', 'volatile', 'wchar_t',
    'while', 'xor', 'xor_eq',

    // Reserved label
    'default:', 'protected:', 'private:', 'public:',

    // C Pre-Processor
    '#define', '#else', '#elif', '#endif', '#error', '#if', '#ifdef', '#ifndef',
    '#include', '#line', '#pragma', '#undef',

    // MSC
    '#import',
    '#using',  // MSC

    // ANSI-C Predefined Macros
    '__DATE__', '__FILE__', '__LINE__', '__STDC__', '__TIME__', '__TIMESTAMP__',

    // MSC Predefined Macros
    '_ATL_VER', '_CHAR_UNSIGNED', '__CLR_VER', '__cplusplus_cli', '__COUNTER__',
    '__cplusplus', '__CPPLIB_VER', '__CPPRTTI', '__CPPUNWIND', '__DEBUG', '_DL',
    '__FUNCDNAME__', '__FUNCSIG__', '__FUNCTION__', '_INTEGRAL_MAX_BITS',
    '_M_ALPHA', '_M_CEE', '_M_CEE_PURE', '_M_CEE_SAFE', '_M_IX86', '_MIA64',
    '_M_IX86_FP', '_M_MPPC', '_M_MRX000', '_M_PPC', '_M_X64', '_MANAGED',
    '_MFC_VER', '_MSC_EXTENSIONS', '_MSC_VER', '_MSVC_RUNTIME_CHECKES', '_MT',
    '_NATIVE_WCHAR_T_DEFINED', '_OPENMP', '_VC_NODEFAULTLIB',
    '_WCHAR_T_DEFINED', '_WIN32', '_WIN64', '_Wp64',

    // <stdint.h>
    'int16_t', 'int32_t', 'int64_t', 'uint8_t', 'uint16_t', 'uint32_t',
    'uint64_t', 'uint8_t',

    // Standard C Library
    'NULL', 'div_t', 'ldiv_t', 'ptrdiff_t', 'size_t', 'va_arg', 'va_end',
    'va_list', 'va_start', 'wchar_t',

    // C++ library
    'std::find', 'std::iterator', 'std::list', 'std::make_unique', 'std::map',
    'std::max', 'std::min', 'std::move', 'std::numeric_limits', 'std::set',
    'std::string', 'std::swap', 'std::unique_ptr', 'std::unordered_map',
    'std::unordered_set', 'std::vector',

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
    'LOWROD', 'WORD', 'interface',
].forEach(word => staticCppKeywords.add(word));

highlights.CppHighlighter = CppHighlighter;
});

// Override |CppLexer| by |CppHighlighter|.
// TODO(eval1749): Once we get rid of |CppLexer|, we should get rid of this
// override.
global['CppLexer'] = highlights.CppHighlighter;
