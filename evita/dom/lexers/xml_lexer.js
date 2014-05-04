// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.XmlLexer = (function(keywords) {
  /** @enum{!Symbol} */
  var XmlState = {
    ATTRNAME: Symbol('attrname'),
    ATTRNAME_EQ: Symbol('attrname='),
    ATTRNAME_EQ_SPACE: Symbol('attrname=space'),
    ATTRNAME_GT: Symbol('attrname>'),
    ATTRNAME_SPACE: Symbol('attrname_space'),
    ATTRVALUE: Symbol('attrvalue'),
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
    ELEMENTNAME_SPACE: Symbol('element_space'),
    ENTITYREF: Symbol('&'),
    GT: Symbol('>'),
    LT: Symbol('<'),
    LT_BANG: Symbol('<!'),
    LT_BANG_DASH: Symbol('<!-'),
    STRING1: Symbol('string1'),
    STRING1_END: Symbol('string1_end'),
    STRING2: Symbol('string2'),
    STRING2_END: Symbol('string2_end'),
    TEXT: Symbol('text')
  };

  /** @const @type {!Map.<XmlState, string>} */
  var stateToSyntax = new Map();
  stateToSyntax.set(XmlState.ATTRNAME, 'html_attribute_name');
  stateToSyntax.set(XmlState.ATTRNAME_EQ, '');
  stateToSyntax.set(XmlState.ATTRNAME_EQ_SPACE, '');
  stateToSyntax.set(XmlState.ATTRNAME_GT, '');
  stateToSyntax.set(XmlState.ATTRNAME_SPACE, '');
  stateToSyntax.set(XmlState.ATTRVALUE, 'string_literal');
  stateToSyntax.set(XmlState.ATTRVALUE_SPACE, '');
  stateToSyntax.set(XmlState.CLOSETAG, '');
  stateToSyntax.set(XmlState.CLOSETAG_SPACE, '');
  stateToSyntax.set(XmlState.COMMENT, 'comment');
  stateToSyntax.set(XmlState.COMMENT_DASH, 'comment');
  stateToSyntax.set(XmlState.COMMENT_DASH_DASH, 'comment');
  stateToSyntax.set(XmlState.COMMENT_END, 'comment');
  stateToSyntax.set(XmlState.COMMENT_START, 'comment');
  stateToSyntax.set(XmlState.ELEMENTNAME, 'html_element_name');
  stateToSyntax.set(XmlState.ELEMENTNAME_SLASH, '');
  stateToSyntax.set(XmlState.ELEMENTNAME_SPACE, '');
  stateToSyntax.set(XmlState.ENTITYREF, 'html_entity');
  stateToSyntax.set(XmlState.GT, '');
  stateToSyntax.set(XmlState.LT, '');
  stateToSyntax.set(XmlState.LT_BANG, '');
  stateToSyntax.set(XmlState.LT_BANG_DASH, '');
  stateToSyntax.set(XmlState.STRING1, 'html_attribute_value');
  stateToSyntax.set(XmlState.STRING1_END, 'html_attribute_value');
  stateToSyntax.set(XmlState.STRING2, 'html_attribute_value');
  stateToSyntax.set(XmlState.STRING2_END, 'html_attribute_value');
  stateToSyntax.set(XmlState.TEXT, '');

  Object.keys(XmlState).forEach(function(key) {
    if (!stateToSyntax.has(XmlState[key]))
      throw new Error('stateToSyntax must have ' + key);
  });

  /** @const @type {!Map.<number, number>} */
  var CHARACTERS = (function() {
    var attrs = new Map();

    attrs.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    attrs.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);

    attrs.set(Unicode.AMPERSAND, Lexer.OPERATOR_CHAR);
    attrs.set(Unicode.APOSTROPHE, Lexer.STRING1_CHAR);
    attrs.set(Unicode.COLON, Lexer.WORD_CHAR);
    attrs.set(Unicode.LESS_THAN_SIGN, Lexer.OPERATOR_CHAR);
    attrs.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    return attrs;
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
      stateToSyntax: stateToSyntax
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
      case XmlState.COMMENT_DASH:
        token.state = XmlState.COMMENT;
        return;
      case XmlState.COMMENT_DASH_DASH:
        token.state = XmlState.COMMENT_DASH;
        return;
      case XmlState.COMMENT_END:
        token.state = XmlState.COMMENT_DASH_DASH;
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
      case XmlState.ELEMENTNAME_SPACE:
        token.state = XmlState.ELEMENTNAME;
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
   * @this {!XmlLexer}
   * @param {!Range} range
   * @param {!Lexer.Token} token
   * @return {string}
   */
  function extractWord(range, token) {
    return range.text;
  }

  /**
   * @this {!XmlLexer}
   * @param {number} maxOffset
   * @return {?Lexer.Token}
   */
  function nextToken(maxOffset) {
    var lexer = this;
    var document = lexer.range.document;
    while (lexer.scanOffset < maxOffset) {
      var charCode = document.charCodeAt_(lexer.scanOffset);
      switch (lexer.state) {
        case Lexer.State.ZERO:
          switch (charCode) {
            case Unicode.AMPERSAND:
              lexer.startToken(XmlState.ENTITYREF);
              continue;
            case Unicode.LESS_THAN_SIGN:
              lexer.startToken(XmlState.LT);
              continue;
            default:
              lexer.startToken(XmlState.TEXT);
              continue;
          }
          break;

        case XmlState.ATTRNAME:
          // attrName '=' | attrName '>' | attrName '/' | attrName space
          if (charCode == Unicode.EQUALS_SIGN)
            return lexer.finishToken(XmlState.ATTRNAME_EQ);
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode))
            return lexer.finishToken(XmlState.ATTRNAME_SPACE);
          lexer.extendToken();
          continue;
        case XmlState.ATTRNAME_EQ:
          // attrName '=' '\'' | attrName '=' '"' | attrName '=' '>' |
          // attrName '=' '/' | attrName '=' space
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.APOSTROPHE) {
            lexer.startToken(XmlState.STRING1);
            continue;
          }
          if (charCode == Unicode.QUOTATION_MARK) {
            lexer.startToken(XmlState.STRING2);
            continue;
          }
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode))
            return lexer.finishToken(XmlState.ATTRNAME_EQ_SPACE);
          lexer.startToken(XmlState.ATTRVALUE);
          continue;
        case XmlState.ATTRNAME_EQ_SPACE:
          if (lexer.isWhitespace(charCode)) {
            lexer.extendToken();
            continue;
          }
          lexer.startToken(XmlState.ATTRVALUE);
          continue;
        case XmlState.ATTRNAME_SPACE:
          // attrName space '=' | attrName space '>' | attrName space '/' |
          // attrName space space | attrName space attrName
          if (charCode == Unicode.EQUALS_SIGN)
            return lexer.finishToken(XmlState.ATTRNAME_EQ);
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode)) {
            lexer.extendToken();
            continue;
          }
          return lexer.finishToken(XmlState.ATTRNAME);

        case XmlState.ATTRVALUE:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode))
            return lexer.finishToken(XmlState.ATTRVALUE_SPACE);
          lexer.extendToken();
          continue;
        case XmlState.ATTRVALUE_SPACE:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode)) {
            lexer.extendToken();
            continue;
          }
          lexer.startToken(XmlState.ATTRNAME);
          continue;

        case XmlState.CLOSETAG:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (lexer.isWhitespace(charCode))
            return lexer.finishToken(XmlState.CLOSETAG_SPACE);
          lexer.startToken(XmlState.ELEMENTNAME);
          continue;
        case XmlState.CLOSETAG_SPACE:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.startToken(XmlState.ELEMENTNAME);
          continue;

        case XmlState.COMMENT:
          if (charCode == Unicode.HYPHEN_MINUS)
            return lexer.finishToken(XmlState.COMMENT_DASH);
          lexer.extendToken();
          continue;
        case XmlState.COMMENT_DASH:
          if (charCode == Unicode.HYPHEN_MINUS)
            return lexer.finishTokenAs(XmlState.COMMENT_DASH_DASH);
          return lexer.finishToken(XmlState.COMMENT);
        case XmlState.COMMENT_DASH_DASH:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishTokenAs(XmlState.COMMENT_END);
          return lexer.finishToken(XmlState.COMMENT);
        case XmlState.COMMENT_END:
          return lexer.finishToken(Lexer.State.ZERO);
        case XmlState.COMMENT_START:
          return lexer.finishToken(XmlState.COMMENT);

        case XmlState.ELEMENTNAME:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode))
            return lexer.finishToken(XmlState.ELEMENTNAME_SPACE);
          lexer.extendToken();
          continue;
        case XmlState.ELEMENTNAME_SLASH:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          lexer.extendToken();
          continue;
        case XmlState.ELEMENTNAME_SPACE:
        case XmlState.STRING1_END:
        case XmlState.STRING2_END:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          if (lexer.isWhitespace(charCode)) {
            if (lexer.state == XmlState.ELEMENTNAME_SPACE)
              lexer.extendToken();
            else
              lexer.startToken(XmlState.ELEMENTNAME_SPACE);
          } else {
            lexer.startToken(XmlState.ATTRNAME);
          }
          continue;

        case XmlState.ENTITYREF:
          // Eat up to semicolon ";"
          lexer.extendToken();
          if (charCode == Unicode.SEMICOLON)
            return lexer.finishToken(Lexer.State.ZERO);
          continue;

        case XmlState.GT:
          return lexer.finishToken(Lexer.State.ZERO);

        case XmlState.LT:
          if (charCode == Unicode.EXCLAMATION_MARK)
            return lexer.finishTokenAs(XmlState.LT_BANG);
          if (charCode == Unicode.GREATER_THAN_SIGN)
            return lexer.finishToken(XmlState.GT);
          if (charCode == Unicode.SOLIDUS)
            return lexer.finishTokenAs(XmlState.CLOSETAG);
          if (charCode == lexer.isWhitespace(charCode))
            return lexer.finishToken(Lexer.State.ZERO);
          lexer.startToken(XmlState.ELEMENTNAME);
          continue;
        case XmlState.LT_BANG:
          if (charCode == Unicode.HYPHEN_MINUS)
            return lexer.finishTokenAs(XmlState.LT_BANG_DASH);
          return lexer.finishToken(XmlState.ELEMENTNAME);
        case XmlState.LT_BANG_DASH:
          if (charCode == Unicode.HYPHEN_MINUS)
            return lexer.finishTokenAs(XmlState.COMMENT_START);
          return lexer.finishToken(XmlState.ELEMENTNAME);

        case XmlState.STRING1:
          // Eat up to single-quote "'"
          if (charCode == Unicode.APOSTROPHE ||
              charCode == Unicode.GREATER_THAN_SIGN) {
            return lexer.finishToken(XmlState.STRING1_END);
          }
          lexer.extendToken();
          continue;

        case XmlState.STRING2:
          // Eat up to double-quote "'"
          if (charCode == Unicode.QUOTATION_MARK ||
              charCode == Unicode.GREATER_THAN_SIGN) {
            return lexer.finishToken(XmlState.STRING2_END);
          }
          lexer.extendToken();
          continue;

        case XmlState.TEXT:
          if (charCode == Unicode.AMPERSAND ||
              charCode == Unicode.LESS_THAN_SIGN) {
            return lexer.finishToken(Lexer.State.ZERO);
          }
          lexer.extendToken();
          continue;
      }
      console.log(lexer);
      throw new Error('Invalid state ' + lexer.state.toString());
    }
    return lexer.lastToken;
  }

  XmlLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: XmlLexer},
    didShrinkLastToken: {value: didShrinkLastToken },
    extractWord: {value: extractWord},
    nextToken: {value: nextToken}
  });

  return XmlLexer;
})([
  'xi:include',
  'xml:base',
  'xml:lang',
  'xmlns:',
  'xml:space'
]);
