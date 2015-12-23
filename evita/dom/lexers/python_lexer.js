// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.PythonLexer = (function(keywords) {
  /** @const @type {!Map.<number, number>} */
  var CHARACTERS = (function() {
    var map = new Map();

    function setRange(type, min, max) {
      for (var charCode = min; charCode <= max; ++charCode) {
        map.set(charCode, type);
      }
    }

    map.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);

    setRange(Lexer.OPERATOR_CHAR, 0x21, 0x3F);
    map.set(Unicode.FULL_STOP, Lexer.DOT_CHAR);

    // String literal
    map.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // NameStartChar ::= [a-zA-Z_]
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_CAPITAL_LETTER_A,
             Unicode.LATIN_CAPITAL_LETTER_Z);
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_SMALL_LETTER_A,
             Unicode.LATIN_SMALL_LETTER_Z);
    map.set(Unicode.LOW_LINE, Lexer.NAMESTART_CHAR);

    // NameChar ::= NameStart | [0-9]
    setRange(Lexer.NAME_CHAR, Unicode.DIGIT_ZERO, Unicode.DIGIT_NINE);

    return map;
  })();

  class PythonLexer extends global.Lexer {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      super(document, {
        characters: CHARACTERS,
        keywords: keywords
      });
    }

    /**
     * @this {!PythonLexer}
     * @param {!lexers.Token} token
     * @return {!lexers.State}
     */
    didShrinkLastToken(token) {
      if (this.debug_ > 1)
        console.log('didShrinkLastToken', token);
      if (token.state == lexers.State.DOT) {
        return lexers.State.ZERO;
      }
      return token.state;
    }

    /**
     * @this {!ConfigLexer}
     * @param {number} charCode
     */
    feedCharacter(charCode) {
      if (this.state == lexers.State.ZERO) {
        switch (charCode) {
          case Unicode.FULL_STOP:
            this.startToken(lexers.State.DOT);
            this.endToken();
            return;
          case Unicode.NUMBER_SIGN:
            this.startToken(lexers.State.LINE_COMMENT);
            return;
        }
      }
      this.updateState(charCode);
    }

    /**
     * @this {!PythonLexer}
     * @param {!lexers.Token} token
     * @param {!Range} range
     * @return {string}
     */
    syntaxOfToken(range, token) {
      if (token.state != lexers.State.WORD)
        return Lexer.prototype.syntaxOfToken.call(this, range, token);
      var lexer = this;
      var word = range.text;
      var it = lexer.tokens.find(token);
      console.assert(it, token);
      do {
        it = it.previous();
      } while (it && it.data.state == lexers.State.SPACE);

      if (it && it.data.state == lexers.State.DOT) {
        var tokens = lexer.collectTokens(it, token);
        return lexer.syntaxOfTokens(range, tokens);
      }

      return lexer.syntaxOfWord(word);
    }
  }

  // TODO(eval1749): Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(PythonLexer, 'keywords', {
    get: function() { return keywords; }
  });
  return PythonLexer;
})(Lexer.createKeywords([
  // Keywords
  'and', 'as', 'assert', 'break', 'class',
  'continue', 'def', 'del', 'elif', 'else', 'except', 'exec',
  'finally', 'for', 'from', 'global', 'if', 'import', 'in', 'is',
  'lambda', 'not', 'or', 'pass', 'print', 'raise', 'return', 'try',
  'while', 'with', 'yield',
  // Reserved classes of identifiers
  '_*', '__*__', '__*', '__builtin__',
  // Built-in Constants
  'Ellipsis', 'False', 'None', 'NotImplemented', 'True', '__debug__',
  // Built-in Types
  'complex', 'float', 'int', 'long',
  'str', 'unicode', 'list', 'tuple', 'bytearray', 'xrange',
  'set', 'frozenset', 'dict',

  // 3.1 Objects, values and types
  '__*item__', '__*slice__',
  '__abs__', '__add__', '__and__',
  '__bases__', '__call__',
  '__class__', '__closure__', '__cmp__', '__code__', '__coerce__',
    '__complex__', '__contains__',
  '__defaults__', '__del__', '__delattr__', '__delete__', '__delitem__',
    '__delslice__', '__dict__', '__div__', '__divmod__', '__doc__',
  '__enter__', '__eq__', '__exit__',
  '__file__', '__float__', '__floordiv__', '__func__', '__future__',
  '__ge__', '__get__', '__getattr__', '__getattribute__', '__getitem__',
    '__getslice__', '__globals__', '__gt__',
  '__hash__', '__hex__',
  '__iadd__', '__iand__', '__idiv__', '__ifloordiv__', '__ilshift__',
    '__imod__', '__imul__', '__index__', '__init__', '__instancecheck__',
    '__int__', '__invert__', '__iop__', '__ior__', '__ipow__', '__irshift__',
    '__isub__', '__iter__', '__itruediv__', '__ixor__',
  '__le__', '__len__', '__long__', '__lshift__', '__lt__',
  '__metaclass__', '__missing__', '__mod__', '__module__', '__mro__',
    '__mul__',
  '__name__', '__ne__', '__neg__', '__new__', '__nonzero__',
  '__oct__', '__op__', '__or__',
  '__pos__', '__pow__',
  '__radd__', '__rand__', '__rcmp__', '__rdiv__', '__rdivmod__', '__repr__',
    '__reversed__', '__rfloordiv__', '__rlshift__', '__rmod__', '__rmul__',
    '__rop__', '__ror__', '__rpow__', '__rrshift__', '__rshift__', '__rsub__',
    '__rtruediv__', '__rxor__',
  '__self__', '__set__', '__setattr__', '__setitem__', '__setslice__',
    '__slots__', '__str__', '__sub__', '__subclasscheck__',
  '__truediv__',
  '__unicode__',
  '__weakref__',
  '__xor__',

  // 5.13. Special Attributes
  '__dict__', '__class__', '__bases__', '__name__', '__mro__',
  '__subclasses__',

  // 28.5 __main__ -- Top-level script environment
  '__main__',

  // Commonly used word
  'self',
]));
