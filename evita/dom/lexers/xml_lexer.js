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

  /** @const @type {!Map.<XmlState, string>} */
  var stateToSyntax = new Map();
  stateToSyntax.set(XmlState.ATTRNAME, 'html_attribute_name');
  stateToSyntax.set(XmlState.ATTRNAME_EQ, '');
  stateToSyntax.set(XmlState.ATTRNAME_GT, '');
  stateToSyntax.set(XmlState.ATTRNAME_SPACE, '');

  stateToSyntax.set(XmlState.ATTRVALUE, 'html_attribute_value');
  stateToSyntax.set(XmlState.ATTRVALUE_END, 'html_attribute_value');
  stateToSyntax.set(XmlState.ATTRVALUE_SPACE, '');
  stateToSyntax.set(XmlState.ATTRVALUE1, 'html_attribute_value');
  stateToSyntax.set(XmlState.ATTRVALUE2, 'html_attribute_value');

  stateToSyntax.set(XmlState.CLOSETAG, '');
  stateToSyntax.set(XmlState.CLOSETAG_SPACE, '');

  stateToSyntax.set(XmlState.COMMENT, 'comment');
  stateToSyntax.set(XmlState.COMMENT_DASH, 'comment');
  stateToSyntax.set(XmlState.COMMENT_DASH_DASH, 'comment');
  stateToSyntax.set(XmlState.COMMENT_END, 'comment');
  stateToSyntax.set(XmlState.COMMENT_START, 'comment');

  stateToSyntax.set(XmlState.ELEMENTNAME, 'html_element_name');
  stateToSyntax.set(XmlState.ELEMENTNAME_SLASH, '');

  stateToSyntax.set(XmlState.ENTITYREF, 'html_entity');
  stateToSyntax.set(XmlState.ENTITYREF_END, 'html_entity');

  stateToSyntax.set(XmlState.GT, '');

  stateToSyntax.set(XmlState.LT, '');
  stateToSyntax.set(XmlState.LT_BANG, '');
  stateToSyntax.set(XmlState.LT_BANG_DASH, '');

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

        ////////////////////////////////////////////////////////////
        //
        // ATTRNAME
        //
        case XmlState.ATTRNAME:
          // attrName '=' | attrName '>' | attrName '/' | attrName space
          if (charCode == Unicode.EQUALS_SIGN)
            lexer.finishToken(XmlState.ATTRNAME_EQ);
          else if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.finishToken(XmlState.ATTRNAME_SPACE);
          else
            lexer.extendToken();
          continue;
        case XmlState.ATTRNAME_EQ:
          // attrName '=' '\'' | attrName '=' '"' | attrName '=' '>' |
          // attrName '=' '/' | attrName '=' space
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.APOSTROPHE)
            lexer.finishToken(XmlState.ATTRVALUE1);
          else if (charCode == Unicode.QUOTATION_MARK)
            lexer.finishToken(XmlState.ATTRVALUE2);
          else if (charCode == Unicode.SOLIDUS)
            lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.finishToken(XmlState.ATTRVALUE);
          continue;
        case XmlState.ATTRNAME_SPACE:
          // attrName space '=' | attrName space '>' | attrName space '/' |
          // attrName space space | attrName space attrName
          if (charCode == Unicode.EQUALS_SIGN)
            lexer.finishToken(XmlState.ATTRNAME_EQ);
          else if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.finishToken(XmlState.ATTRNAME);
          continue;

        ////////////////////////////////////////////////////////////
        //
        // ATTRVALUE
        //
        case XmlState.ATTRVALUE:
          // attribute value without quote
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.finishToken(XmlState.ATTRVALUE_SPACE);
          else
            lexer.extendToken();
          continue;

        case XmlState.ATTRVALUE1:
          // Eat up to single-quote "'". ">" terminates attribute value.
          if (charCode == Unicode.APOSTROPHE)
            lexer.finishToken(XmlState.ATTRVALUE_END);
          else if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else
            lexer.extendToken();
          continue;

        case XmlState.ATTRVALUE2:
          // Eat up to double-quote "\"". ">" terminates attribute value.
          if (charCode == Unicode.QUOTATION_MARK)
            lexer.finishToken(XmlState.ATTRVALUE_END);
          else if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else
            lexer.extendToken();
          continue;

        case XmlState.ATTRVALUE_END:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (lexer.isWhitespace(charCode))
            lexer.finishToken(XmlState.ATTRVALUE_SPACE);
          else
            lexer.finishToken(XmlState.ATTRNAME);
          continue;

        case XmlState.ATTRVALUE_SPACE:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            return lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.finishToken(XmlState.ATTRNAME);
          continue;

        ////////////////////////////////////////////////////////////
        //
        // CLOSETAG
        //
        case XmlState.CLOSETAG:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (lexer.isWhitespace(charCode))
            lexer.finishToken(XmlState.CLOSETAG_SPACE);
          else
            lexer.finishToken(XmlState.ELEMENTNAME);
          continue;
        case XmlState.CLOSETAG_SPACE:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (lexer.isWhitespace(charCode))
            lexer.extendToken();
          else
            lexer.finishToken(XmlState.ELEMENTNAME);
          continue;

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
            lexer.finishToken(XmlState.COMMENT_DASH);
          else
            lexer.extendToken();
          continue;
        case XmlState.COMMENT_DASH:
          if (charCode == Unicode.HYPHEN_MINUS)
            lexer.restartToken(XmlState.COMMENT_DASH_DASH);
          else
            lexer.finishToken(XmlState.COMMENT);
            continue;
        case XmlState.COMMENT_DASH_DASH:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.restartToken(XmlState.COMMENT_END);
          else
            lexer.finishToken(XmlState.COMMENT);
          continue;
        case XmlState.COMMENT_END:
          lexer.endToken();
          continue;
        case XmlState.COMMENT_START:
          lexer.finishToken(XmlState.COMMENT);
          continue;

        ////////////////////////////////////////////////////////////
        //
        // ELEMENTNAME
        //
        case XmlState.ELEMENTNAME:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            lexer.finishToken(XmlState.ELEMENTNAME_SLASH);
          else if (lexer.isWhitespace(charCode))
            lexer.finishToken(XmlState.ATTRVALUE_SPACE);
          else
            lexer.extendToken();
          continue;
        case XmlState.ELEMENTNAME_SLASH:
          if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else
            lexer.extendToken();
          continue;

        ////////////////////////////////////////////////////////////
        //
        // ENTITYREF
        //
        case XmlState.ENTITYREF:
          // Eat up to semicolon ";"
          if (charCode == Unicode.SEMICOLON)
            lexer.finishToken(XmlState.ENTITYREF_END);
          else
            lexer.extendToken();
          continue;

        case XmlState.ENTITYREF_END:
        case XmlState.GT:
          lexer.endToken();
          continue;

        ////////////////////////////////////////////////////////////
        //
        // LT
        //
        case XmlState.LT:
          if (charCode == Unicode.EXCLAMATION_MARK)
            lexer.restartToken(XmlState.LT_BANG);
          else if (charCode == Unicode.GREATER_THAN_SIGN)
            lexer.finishToken(XmlState.GT);
          else if (charCode == Unicode.SOLIDUS)
            lexer.restartToken(XmlState.CLOSETAG);
          else if (charCode == lexer.isWhitespace(charCode))
            lexer.endToken();
          else
            lexer.finishToken(XmlState.ELEMENTNAME);
          continue;
        case XmlState.LT_BANG:
          if (charCode == Unicode.HYPHEN_MINUS)
            lexer.restartToken(XmlState.LT_BANG_DASH);
          else
            lexer.finishToken(XmlState.ELEMENTNAME);
          continue;
        case XmlState.LT_BANG_DASH:
          if (charCode == Unicode.HYPHEN_MINUS)
            lexer.restartToken(XmlState.COMMENT_START);
          else
            lexer.finishToken(XmlState.ELEMENTNAME);
          continue;

        case XmlState.TEXT:
          if (charCode == Unicode.AMPERSAND)
            lexer.endToken();
          else if (charCode == Unicode.LESS_THAN_SIGN)
            lexer.endToken();
          else
            lexer.extendToken();
          continue;
      }
      console.log(lexer);
      throw new Error('Invalid state ' + lexer.state.toString());
    }
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
