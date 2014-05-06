// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(yosi) Should we color entity reference in attribute value? VS2013
// doesn't color.
// TODO(yosi) NYI: <![CDATA[ ... ]]>
global.XmlLexer = (function(keywords) {
  /** @enum{!Symbol} */
  // ATTRVALUE_END and ENTITYREF_END make attribute value/entity reference
  // coloring with backspace.
  //
  //  Before: ATTRNAME = ATTRVALUE ATTRVALUE_END CHAR
  //  Action: hit backspace after CHAR
  //  After: ATTRNAME = ATTRVALUE ATTRVALUE_END
  //  Scanning starts from ATTRVALUE_END
  var XmlState = {
    ATTRNAME: Symbol('attrname'),
    ATTRNAME_EQ: Symbol('attrname='),
    ATTRNAME_GT: Symbol('attrname>'),
    ATTRNAME_SPACE: Symbol('attrname_space'),
    ATTRVALUE: Symbol('attrvalue'),
    ATTRVALUE1: Symbol('string1'),
    ATTRVALUE2: Symbol('string2'),
    ATTRVALUE_END: Symbol('attrvalue_end'),
    ATTRVALUE_SPACE: Symbol('attrvalue_space'),
    CLOSETAG: Symbol('</'),
    CLOSETAG_SPACE: Symbol('</_space'),
    COMMENT: Symbol('comment'),
    COMMENT_DASH: Symbol('comment-'),
    COMMENT_DASH_DASH: Symbol('comment--'),
    COMMENT_END: Symbol('-->'),
    COMMENT_START: Symbol('<!--'),
    ELEMENTNAME: Symbol('element'),
    ELEMENTNAME_SLASH: Symbol('element/'),
    ENTITYREF: Symbol('&'),
    ENTITYREF_END: Symbol('&;'),
    GT: Symbol('>'),
    LT: Symbol('<'),
    LT_BANG: Symbol('<!'),
    LT_BANG_DASH: Symbol('<!-'),
    TEXT: Symbol('text')
  };

  /** @const @type {!Map.<!XmlState, string>} */
  var STATE_TO_SYNTAX = (function() {
    var map = new Map();
    map.set(XmlState.ATTRNAME, 'html_attribute_name');
    map.set(XmlState.ATTRNAME_EQ, '');
    map.set(XmlState.ATTRNAME_GT, '');
    map.set(XmlState.ATTRNAME_SPACE, '');

    map.set(XmlState.ATTRVALUE, 'html_attribute_value');
    map.set(XmlState.ATTRVALUE_END, 'html_attribute_value');
    map.set(XmlState.ATTRVALUE_SPACE, '');
    map.set(XmlState.ATTRVALUE1, 'html_attribute_value');
    map.set(XmlState.ATTRVALUE2, 'html_attribute_value');

    map.set(XmlState.CLOSETAG, '');
    map.set(XmlState.CLOSETAG_SPACE, '');

    map.set(XmlState.COMMENT, 'comment');
    map.set(XmlState.COMMENT_DASH, 'comment');
    map.set(XmlState.COMMENT_DASH_DASH, 'comment');
    map.set(XmlState.COMMENT_END, 'comment');
    map.set(XmlState.COMMENT_START, 'comment');

    map.set(XmlState.ELEMENTNAME, 'html_element_name');
    map.set(XmlState.ELEMENTNAME_SLASH, '');

    map.set(XmlState.ENTITYREF, 'html_entity');
    map.set(XmlState.ENTITYREF_END, 'html_entity');

    map.set(XmlState.GT, '');

    map.set(XmlState.LT, '');
    map.set(XmlState.LT_BANG, '');
    map.set(XmlState.LT_BANG_DASH, '');

    map.set(XmlState.TEXT, '');

    Object.keys(XmlState).forEach(function(key) {
      if (!map.has(XmlState[key]))
        throw new Error('map must have ' + key);
    });
    return map;
  })();

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

    map.set(Unicode.AMPERSAND, Lexer.OPERATOR_CHAR);
    map.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    map.set(Unicode.COLON, Lexer.NAMESTART_CHAR);
    map.set(Unicode.LESS_THAN_SIGN, Lexer.OPERATOR_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // NameStartChar ::= [A-Z] | [a-z] | '_' | ':' | ...
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_CAPITAL_LETTER_A,
             Unicode.LATIN_CAPITAL_LETTER_Z);
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_SMALL_LETTER_A,
             Unicode.LATIN_SMALL_LETTER_Z);
    map.set(Unicode.LOW_LINE, Lexer.NAMESTART_CHAR);
    map.set(Unicode.COLON, Lexer.NAMESTART_CHAR);
    setRange(Lexer.NAME_CHAR, 0x00C0, 0x00D6);
    setRange(Lexer.NAME_CHAR, 0x00D8, 0x00F6);
    setRange(Lexer.NAME_CHAR, 0x00F8, 0x02FF);
    setRange(Lexer.NAME_CHAR, 0x0370, 0x037D);
    setRange(Lexer.NAME_CHAR, 0x037F, 0x1FFF);
    setRange(Lexer.NAME_CHAR, 0x200C, 0x200D);
    setRange(Lexer.NAME_CHAR, 0x2070, 0x218F);
    setRange(Lexer.NAME_CHAR, 0x2C00, 0x2FEF);
    setRange(Lexer.NAME_CHAR, 0x3001, 0xD7FF);
    setRange(Lexer.NAME_CHAR, 0xF900, 0xFDCF);
    setRange(Lexer.NAME_CHAR, 0xFDF0, 0xFFFD);

    // NameChar ::= [09-9] | '.' | '-' | ...
    setRange(Lexer.NAME_CHAR, Unicode.DIGIT_ZERO, Unicode.DIGIT_NINE);
    setRange(Lexer.NAME_CHAR, 0x0300, 0x036F);
    setRange(Lexer.NAME_CHAR, 0x203F, 0x2040);
    map.set(Unicode.FULL_STOP, Lexer.NAME_CHAR);
    map.set(Unicode.HYPHEN_MINUS, Lexer.NAME_CHAR);
    map.set(0xB7, Lexer.NAME_CHAR);

    return map;
  })();

  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function XmlLexer(document) {
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: keywords,
    });
  }

  /**
   * @this {!XmlLexer}
   * @param {!Lexer.Token} token
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    switch (token.state) {
      case XmlState.COMMENT_DASH_DASH:
        token.state = XmlState.COMMENT_DASH;
        return;
      case XmlState.COMMENT_END:
        switch (token.end - token.start) {
          case 1:
            token.state = XmlState.COMMENT_DASH;
            break;
          case 2:
            token.state = XmlState.COMMENT_DASH_DASH;
            break;
          default:
           console.assert(false, token);
           break;
        }
        return;
      case XmlState.COMMENT_START:
        switch (token.end - token.start) {
          case 1:
            token.state = XmlState.LT;
            break;
          case 2:
            token.state = XmlState.LT_BANG;
            break;
          case 3:
            token.state = XmlState.LT_BANG_DASH;
            break;
          default:
            console.assert(false, token);
            break;
        }
        return;
      case XmlState.LT_BANG:
        token.state = XmlState.LT;
        return;
      case XmlState.LT_BANG_DASH:
        token.state = XmlState.LT_BANG;
        return;
    }
  }

  /**
   * @this {!ConfigLexer}
   * @param {number} charCode
   */
  function feedCharacter(charCode) {
    switch (this.state) {
      case Lexer.State.ZERO:
        switch (charCode) {
          case Unicode.AMPERSAND:
            this.startToken(XmlState.ENTITYREF);
            return;
          case Unicode.LESS_THAN_SIGN:
            this.startToken(XmlState.LT);
            return;
          default:
            this.startToken(XmlState.TEXT);
            return;
        }
        break;

      ////////////////////////////////////////////////////////////
      //
      // ATTRNAME
      //
      case XmlState.ATTRNAME:
        // attrName '=' | attrName '>' | attrName '/' | attrName space
        if (charCode == Unicode.EQUALS_SIGN)
          this.finishToken(XmlState.ATTRNAME_EQ);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlState.ATTRNAME_SPACE);
        else
          this.extendToken();
        return;
      case XmlState.ATTRNAME_EQ:
        // attrName '=' '\'' | attrName '=' '"' | attrName '=' '>' |
        // attrName '=' '/' | attrName '=' space
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlState.ATTRVALUE1);
        else if (charCode == Unicode.QUOTATION_MARK)
          this.finishToken(XmlState.ATTRVALUE2);
        else if (charCode == Unicode.SOLIDUS)
          this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          this.finishToken(XmlState.ATTRVALUE);
        return;
      case XmlState.ATTRNAME_SPACE:
        // attrName space '=' | attrName space '>' | attrName space '/' |
        // attrName space space | attrName space attrName
        if (charCode == Unicode.EQUALS_SIGN)
          this.finishToken(XmlState.ATTRNAME_EQ);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          this.finishToken(XmlState.ATTRNAME);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTRVALUE
      //
      case XmlState.ATTRVALUE:
        // attribute value without quote
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlState.ATTRVALUE_SPACE);
        else
          this.extendToken();
        return;

      case XmlState.ATTRVALUE1:
        // Eat up to single-quote "'". ">" terminates attribute value.
        if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlState.ATTRVALUE_END);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else
          this.extendToken();
        return;

      case XmlState.ATTRVALUE2:
        // Eat up to double-quote "\"". ">" terminates attribute value.
        if (charCode == Unicode.QUOTATION_MARK)
          this.finishToken(XmlState.ATTRVALUE_END);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else
          this.extendToken();
        return;

      case XmlState.ATTRVALUE_END:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlState.ATTRVALUE_SPACE);
        else
          this.finishToken(XmlState.ATTRNAME);
        return;

      case XmlState.ATTRVALUE_SPACE:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          return this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          this.finishToken(XmlState.ATTRNAME);
        return;

      ////////////////////////////////////////////////////////////
      //
      // CLOSETAG
      //
      case XmlState.CLOSETAG:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlState.CLOSETAG_SPACE);
        else
          this.finishToken(XmlState.ELEMENTNAME);
        return;
      case XmlState.CLOSETAG_SPACE:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          this.finishToken(XmlState.ELEMENTNAME);
        return;

      ////////////////////////////////////////////////////////////
      //
      // COMMENT
      //    COMMENT_STAT "<!--"
      //    COMMENT characters
      //    COMMENT_DASH "-"
      //    COMMENT_DASH_DASH "--"
      //    COMMENT_END "-->"
      //
      case XmlState.COMMENT:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.finishToken(XmlState.COMMENT_DASH);
        else
          this.extendToken();
        return;
      case XmlState.COMMENT_DASH:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlState.COMMENT_DASH_DASH);
        else
          this.finishToken(XmlState.COMMENT);
          return;
      case XmlState.COMMENT_DASH_DASH:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.restartToken(XmlState.COMMENT_END);
        else
          this.finishToken(XmlState.COMMENT);
        return;
      case XmlState.COMMENT_END:
        this.endToken();
        return;
      case XmlState.COMMENT_START:
        this.finishToken(XmlState.COMMENT);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ELEMENTNAME
      //
      case XmlState.ELEMENTNAME:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.finishToken(XmlState.ELEMENTNAME_SLASH);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlState.ATTRVALUE_SPACE);
        else
          this.extendToken();
        return;
      case XmlState.ELEMENTNAME_SLASH:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else
          this.extendToken();
        return;

      ////////////////////////////////////////////////////////////
      //
      // ENTITYREF
      //
      case XmlState.ENTITYREF:
        // Eat up to semicolon ";"
        if (charCode == Unicode.SEMICOLON)
          this.finishToken(XmlState.ENTITYREF_END);
        else
          this.extendToken();
        return;

      case XmlState.ENTITYREF_END:
      case XmlState.GT:
        this.endToken();
        return;

      ////////////////////////////////////////////////////////////
      //
      // LT
      //
      case XmlState.LT:
        if (charCode == Unicode.EXCLAMATION_MARK)
          this.restartToken(XmlState.LT_BANG);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlState.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(XmlState.CLOSETAG);
        else if (charCode == this.isWhitespace(charCode))
          this.endToken();
        else
          this.finishToken(XmlState.ELEMENTNAME);
        return;
      case XmlState.LT_BANG:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlState.LT_BANG_DASH);
        else
          this.finishToken(XmlState.ELEMENTNAME);
        return;
      case XmlState.LT_BANG_DASH:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlState.COMMENT_START);
        else
          this.finishToken(XmlState.ELEMENTNAME);
        return;

      case XmlState.TEXT:
        if (charCode == Unicode.AMPERSAND)
          this.endToken();
        else if (charCode == Unicode.LESS_THAN_SIGN)
          this.endToken();
        else
          this.extendToken();
        return;
    }
    console.log(this);
    throw new Error('Invalid state ' + this.state.toString());
  }

  /**
   * @this {!XmlLexer}
   * @param {!Lexer.Token} token
   * @param {!Range} range
   * @return {string}
   */
  function syntaxOfToken(range, token) {
    return STATE_TO_SYNTAX.get(token.state) || '';
  }

  XmlLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: XmlLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    feedCharacter: {value: feedCharacter},
    syntaxOfToken: {value: syntaxOfToken}
  });

  return XmlLexer;
})(Lexer.createKeywords([
  'xi:include',
  'xml:base',
  'xml:lang',
  'xmlns:',
  'xml:space'
]));
