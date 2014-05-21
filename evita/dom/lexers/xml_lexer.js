// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(yosi) Should we color entity reference in attribute value? VS2013
// doesn't color.
// TODO(yosi) NYI: <![CDATA[ ... ]]>
global.XmlLexer = (function(xmlOptions) {

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
   * @param {!XmlLexerOptions=} opt_options
   */
  function XmlLexer(document, opt_options) {
    var options = arguments.length >= 2 ? opt_options : xmlOptions;
    Lexer.call(this, document, {
      characters: CHARACTERS,
      keywords: options.keywords,
    });
    this.hasScript = options.hasScript;
  }

  /** @enum{!Symbol} */
  // ATTRVALUE_END and AMPERSAND_END make attribute value/entity reference
  // coloring with backspace.
  //
  //  Before: ATTRNAME = ATTRVALUE ATTRVALUE_END CHAR
  //  Action: hit backspace after CHAR
  //  After: ATTRNAME = ATTRVALUE ATTRVALUE_END
  //  Scanning starts from ATTRVALUE_END
  XmlLexer.State = {
    AMPERSAND: Symbol('&'),
    AMPERSAND_END: Symbol('&;'),

    ATTRNAME: Symbol('attrname'),
    ATTRNAME_EQ: Symbol('attrname='),
    ATTRNAME_SPACE: Symbol('attrname_space'),

    ATTROTHER: Symbol('attrother'),

    ATTRVALUE: Symbol('attrvalue'),
    ATTRVALUE1: Symbol('attrvalue1'),
    ATTRVALUE1_START: Symbol('attrvalue1_start'),
    ATTRVALUE2: Symbol('attrvalue2'),
    ATTRVALUE2_START: Symbol('attrvalue2_start'),
    ATTRVALUE_END: Symbol('attrvalue_end'),
    ATTRVALUE_SPACE: Symbol('attrvalue_space'),

    CHARREF_DIGIT: Symbol('&#d'),
    CHARREF: Symbol('&#'),
    CHARREF_XDIGIT: Symbol('&#x'),

    CLOSETAG: Symbol('</'),
    CLOSETAG_SPACE: Symbol('</_space'),

    COMMENT: Symbol('comment'),
    COMMENT_DASH: Symbol('comment-'),
    COMMENT_DASH_DASH: Symbol('comment--'),
    COMMENT_END: Symbol('-->'),
    COMMENT_START: Symbol('<!--'),

    ELEMENTNAME: Symbol('element'),

    EMPTYTAG_END: Symbol('/>'),

    ENTITYREF: Symbol('entityref'),

    GT: Symbol('>'),
    LT: Symbol('<'),
    LT_BANG: Symbol('<!'),
    LT_BANG_DASH: Symbol('<!-'),
    LT_QUESTION: Symbol('<?'),
    SLASH: Symbol('element/'),
    TEXT: Symbol('text')
  };

  /** @const @type {!Map.<!XmlLexer.State, string>} */
  var STATE_TO_SYNTAX = (function() {
    var map = new Map();
    map.set(XmlLexer.State.AMPERSAND, '');
    map.set(XmlLexer.State.AMPERSAND_END, 'html_entity');

    map.set(XmlLexer.State.ATTRNAME, 'html_attribute_name');
    map.set(XmlLexer.State.ATTRNAME_EQ, '');
    map.set(XmlLexer.State.ATTRNAME_SPACE, '');

    map.set(XmlLexer.State.ATTROTHER, '')

    map.set(XmlLexer.State.ATTRVALUE, 'html_attribute_value');
    map.set(XmlLexer.State.ATTRVALUE_END, '');
    map.set(XmlLexer.State.ATTRVALUE_SPACE, '');
    map.set(XmlLexer.State.ATTRVALUE1, 'html_attribute_value');
    map.set(XmlLexer.State.ATTRVALUE1_START, '');
    map.set(XmlLexer.State.ATTRVALUE2, 'html_attribute_value');
    map.set(XmlLexer.State.ATTRVALUE2_START, '');

    map.set(XmlLexer.State.CHARREF, 'html_entity');
    map.set(XmlLexer.State.CHARREF_DIGIT, 'html_entity');
    map.set(XmlLexer.State.CHARREF_XDIGIT, 'html_entity');

    map.set(XmlLexer.State.CLOSETAG, 'keyword');
    map.set(XmlLexer.State.CLOSETAG_SPACE, '');

    map.set(XmlLexer.State.COMMENT, 'comment');
    map.set(XmlLexer.State.COMMENT_DASH, 'comment');
    map.set(XmlLexer.State.COMMENT_DASH_DASH, 'comment');
    map.set(XmlLexer.State.COMMENT_END, 'comment');
    map.set(XmlLexer.State.COMMENT_START, 'comment');

    map.set(XmlLexer.State.ELEMENTNAME, 'html_element_name');

    map.set(XmlLexer.State.EMPTYTAG_END, 'keyword');

    map.set(XmlLexer.State.ENTITYREF, 'html_entity');

    map.set(XmlLexer.State.GT, 'keyword');

    map.set(XmlLexer.State.LT, 'keyword');
    map.set(XmlLexer.State.LT_BANG, '');
    map.set(XmlLexer.State.LT_BANG_DASH, '');
    map.set(XmlLexer.State.LT_QUESTION, 'keyword');

    map.set(XmlLexer.State.SLASH, '');
    map.set(XmlLexer.State.TEXT, '');

    Object.keys(XmlLexer.State).forEach(function(key) {
      if (!map.has(XmlLexer.State[key]))
        throw new Error('map must have ' + key);
    });
    return map;
  })();

  /**
   * @this {!XmlLexer}
   * @param {!Lexer.Token} token
   * @return {!Lexer.State}
   */
  function didShrinkLastToken(token) {
    if (this.debug_ > 1)
      console.log('didShrinkLastToken', token);
    switch (token.state) {
      case XmlLexer.State.CHARREF:
        return XmlLexer.State.AMPERSAND;
      case XmlLexer.State.COMMENT_DASH_DASH:
        return XmlLexer.State.COMMENT_DASH;
      case XmlLexer.State.COMMENT_END:
        switch (token.end - token.start) {
          case 1:
            return XmlLexer.State.COMMENT_DASH;
          case 2:
            return XmlLexer.State.COMMENT_DASH_DASH;
          default:
           console.assert(false, token);
           break;
        }
        break;
      case XmlLexer.State.COMMENT_START:
        switch (token.end - token.start) {
          case 1:
            return XmlLexer.State.LT;
          case 2:
            return XmlLexer.State.LT_BANG;
          case 3:
            return XmlLexer.State.LT_BANG_DASH;
          default:
            console.assert(false, token);
            break;
        }
        break;
      case XmlLexer.State.EMPTYTAG_END:
        return XmlLexer.State.SLASH;
      case XmlLexer.State.ENTITYREF:
        if (token.end - token.start == 1)
          return XmlLexer.State.AMPERSAND;
        break;
      case XmlLexer.State.LT_BANG:
        return XmlLexer.State.LT;
      case XmlLexer.State.LT_BANG_DASH:
        return XmlLexer.State.LT_BANG;
    }
    return token.state;
  }

  /**
   * @this {!XmlLexer}
   * @param {number} charCode
   */
  function feedCharacter(charCode) {
    switch (this.state) {
      case Lexer.State.ZERO:
        switch (charCode) {
          case Unicode.AMPERSAND:
            this.startToken(XmlLexer.State.AMPERSAND);
            return;
          case Unicode.LESS_THAN_SIGN:
            this.startToken(XmlLexer.State.LT);
            return;
          default:
            this.startToken(XmlLexer.State.TEXT);
            return;
        }
        break;

      ////////////////////////////////////////////////////////////
      //
      // AMPERSAND
      //
      case XmlLexer.State.AMPERSAND:
        if (charCode == Unicode.NUMBER_SIGN)
          this.restartToken(XmlLexer.State.CHARREF);
        else if (this.isNameStart(charCode))
          this.restartToken(XmlLexer.State.ENTITYREF);
        else
          this.state = Lexer.State.ZERO;
        return;

      case XmlLexer.State.CHARREF:
        if (charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE)
          this.finishToken(XmlLexer.State.CHARREF_DIGIT);
        else if (charCode == Unicode.LATIN_SMALL_LETTER_X)
          this.finishToken(XmlLexer.State.CHARREF_XDIGIT);
        else
          this.endToken();
        return;

      case XmlLexer.State.ENTITYREF:
      case XmlLexer.State.CHARREF_DIGIT:
      case XmlLexer.State.CHARREF_XDIGIT:
        // Eat up to semicolon ";"
        if (charCode == Unicode.SEMICOLON || !this.isName(charCode))
          this.finishToken(XmlLexer.State.AMPERSAND_END);
        else
          this.extendToken();
        return;

      case XmlLexer.State.AMPERSAND_END:
      case XmlLexer.State.EMPTYTAG_END:
      case XmlLexer.State.GT:
        this.endToken();
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTRNAME
      //
      case XmlLexer.State.ATTRNAME:
        // attrName '=' | attrName '>' | attrName '/' | attrName space
        if (charCode == Unicode.EQUALS_SIGN)
          this.finishToken(XmlLexer.State.ATTRNAME_EQ);
        else if (this.isName(charCode))
          this.extendToken();
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME_SPACE);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;
      case XmlLexer.State.ATTRNAME_EQ:
        // attrName '=' '\'' | attrName '=' '"' | attrName '=' space
        if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE1_START);
        else if (charCode == Unicode.QUOTATION_MARK)
          this.finishToken(XmlLexer.State.ATTRVALUE2_START);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          processElementTag(this, charCode, XmlLexer.State.ATTRVALUE);
        return;
      case XmlLexer.State.ATTRNAME_SPACE:
        // attrName space '=' | attrName space attrName
        if (charCode == Unicode.EQUALS_SIGN)
          this.finishToken(XmlLexer.State.ATTRNAME_EQ);
        else if (this.isNameStart(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTROTHER
      //
      case XmlLexer.State.ATTROTHER:
        if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTRVALUE
      //
      case XmlLexer.State.ATTRVALUE:
        // attribute value without quote
        if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTRVALUE);
        return;

      case XmlLexer.State.ATTRVALUE1:
        // Eat up to single-quote "'". ">" terminates attribute value.
        if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else
          this.extendToken();
        return;

      case XmlLexer.State.ATTRVALUE1_START:
        if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else
          this.finishToken(XmlLexer.State.ATTRVALUE1);
        return;

      case XmlLexer.State.ATTRVALUE2:
        // Eat up to double-quote "\"". ">" terminates attribute value.
        if (charCode == Unicode.QUOTATION_MARK)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else
          this.extendToken();
        return;

      case XmlLexer.State.ATTRVALUE2_START:
        if (charCode == Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else
          this.finishToken(XmlLexer.State.ATTRVALUE2);
        return;

      case XmlLexer.State.ATTRVALUE_END:
        if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else if (this.isNameStart(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      case XmlLexer.State.ATTRVALUE_SPACE:
        if (this.isWhitespace(charCode))
          this.extendToken();
        else if (this.isNameStart(charCode))
           this.finishToken(XmlLexer.State.ATTRNAME);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // CLOSETAG
      //
      case XmlLexer.State.CLOSETAG:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.CLOSETAG_SPACE);
        else
          this.finishToken(XmlLexer.State.ELEMENTNAME);
        return;
      case XmlLexer.State.CLOSETAG_SPACE:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (this.isWhitespace(charCode))
          this.extendToken();
        else
          this.finishToken(XmlLexer.State.ELEMENTNAME);
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
      case XmlLexer.State.COMMENT:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.finishToken(XmlLexer.State.COMMENT_DASH);
        else
          this.extendToken();
        return;
      case XmlLexer.State.COMMENT_DASH:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.COMMENT_DASH_DASH);
        else
          this.finishToken(XmlLexer.State.COMMENT);
          return;
      case XmlLexer.State.COMMENT_DASH_DASH:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.restartToken(XmlLexer.State.COMMENT_END);
        else
          this.finishToken(XmlLexer.State.COMMENT);
        return;
      case XmlLexer.State.COMMENT_END:
        this.endToken();
        return;
      case XmlLexer.State.COMMENT_START:
        this.finishToken(XmlLexer.State.COMMENT);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ELEMENTNAME
      //
      case XmlLexer.State.ELEMENTNAME:
        if (this.isName(charCode))
          this.extendToken();
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // LT
      //
      case XmlLexer.State.LT:
        if (charCode == Unicode.EXCLAMATION_MARK)
          this.restartToken(XmlLexer.State.LT_BANG);
        else if (charCode == Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (charCode == Unicode.SOLIDUS)
          this.restartToken(XmlLexer.State.CLOSETAG);
        else if (this.isNameStart(charCode))
          this.finishToken(XmlLexer.State.ELEMENTNAME);
        else if (charCode == Unicode.QUESTION_MARK)
          this.restartToken(XmlLexer.State.LT_QUESTION);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_BANG:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.LT_BANG_DASH);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_BANG_DASH:
        if (charCode == Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.COMMENT_START);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_QUESTION:
        if (this.isNameStart(charCode))
          this.finishToken(XmlLexer.State.ELEMENTNAME);
        else
          this.endToken();
        return;

      ////////////////////////////////////////////////////////////
      //
      // SLASH
      //
      case XmlLexer.State.SLASH:
        if (charCode == Unicode.GREATER_THAN_SIGN)
          this.restartToken(XmlLexer.State.EMPTYTAG_END);
        else if (this.isWhitespace(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processElementTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // TEXT
      //
      case XmlLexer.State.TEXT:
        if (charCode == Unicode.AMPERSAND)
          this.finishToken(XmlLexer.State.AMPERSAND);
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
   * @param {!XmlLexer} lexer
   * @param {number} charCode
   * @param {!XmlLexer.State} defaultState
   */
  function processElementTag(lexer, charCode, defaultState) {
    if (charCode == Unicode.GREATER_THAN_SIGN)
      lexer.finishToken(XmlLexer.State.GT);
    else if (charCode == Unicode.QUESTION_MARK)
      lexer.finishToken(XmlLexer.State.SLASH);
    else if (charCode == Unicode.SOLIDUS)
      lexer.finishToken(XmlLexer.State.SLASH);
    else if (lexer.state == defaultState)
      lexer.extendToken();
    else
      lexer.finishToken(defaultState);
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
})({
  hasScript: false,
  keywords: Lexer.createKeywords([
      'xi:include',
      'xml:base',
      'xml:lang',
      'xmlns:',
      'xml:space'
])});
