// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(eval1749): Should we color entity reference in attribute value? VS2013
// doesn't color.
// TODO(eval1749): NYI: <![CDATA[ ... ]]>
(function(xmlOptions) {

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

  //////////////////////////////////////////////////////////////////////
  //
  // XmlLexer
  //
  class XmlLexer extends global.Lexer {
    /**
     * @param {!Document} document
     * @param {!XmlLexerOptions=} opt_options
     */
    constructor(document, opt_options) {
      var options = arguments.length >= 2 ? opt_options : xmlOptions;
      super(document, {
        characters: CHARACTERS,
        keywords: options['keywords'],
      });
      const lexer = this;
      lexer.ignoreCase_ = Boolean(options.ignoreCase);
      var childLexers = options['childLexers'] || {};
      lexer.childLexerMap_ = new Map();
      Object.keys(childLexers).forEach(function(tagName) {
        /** @type {!Lexer} */
        var childLexer = new childLexers[tagName](document, lexer);
        lexer.childLexerMap_.set(tagName, childLexer);
      });
    }

    /**
     * @override
     * @param {number} changedOffset
     */
    adjustScanOffset(changedOffset) {
      super.adjustScanOffset(changedOffset);
      this.childLexerMap_.forEach(childLexer => {
        childLexer.adjustScanOffset(changedOffset);
      });
    }

    /**
     * @override
     */
    clear() {
      Lexer.prototype.clear.call(this);
      this.childLexerMap_.forEach(function(childLexer) {
        childLexer.clear();
      });
    }

    /**
     * @override
     * @param {!Lexer.Token} token
     */
    colorToken(token) {
      if (token.state === XmlLexer.State.SCRIPT)
        return;
      if (token.state !== XmlLexer.State.SCRIPT_END) {
        Lexer.prototype.colorToken.call(this, token);
        return;
      }
      // Color "</script>" as "</" + "script" + ">"
      var range = this.range;
      range.collapseTo(token.start);
      range.end = token.start + 2;
      range.setSyntax('keyword');
      range.collapseTo(token.start + 2);
      range.end = token.end - 1;
      range.setSyntax('html_element_name');
      range.collapseTo(token.end - 1);
      range.end = token.end;
      range.setSyntax('keyword');
    }

    /**
     * @override
     * @param {!Lexer.Token} token
     * @param {!Range} range
     * @return {string}
     */
    syntaxOfToken(range, token) {
      return STATE_TO_SYNTAX.get(token.state) || '';
    }
  }

  /**
   * @type {!Map.<string, !Lexer>}
   */
  XmlLexer.prototype.childLexerMap_;

  /**
   * @type {boolean}
   */
  XmlLexer.prototype.ignoreCase_;

  /** @enum{!symbol} */
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

    COMMENT: Symbol('comment'),
    COMMENT_DASH: Symbol('comment-'),
    COMMENT_DASH_DASH: Symbol('comment--'),
    COMMENT_END: Symbol('-->'),
    COMMENT_START: Symbol('<!--'),

    EMPTYTAG_END: Symbol('/>'),

    ENDTAG: Symbol('</'),
    ENDTAG_NAME: Symbol('</name'),
    ENDTAG_OTHER: Symbol('</other'),
    ENDTAG_SPACE: Symbol('</space'),

    ENTITYREF: Symbol('entityref'),

    GT: Symbol('>'),
    LT: Symbol('<'),
    LT_BANG: Symbol('<!'),
    LT_BANG_DASH: Symbol('<!-'),
    LT_QUESTION: Symbol('<?'),

    SCRIPT: Symbol('script'),
    SCRIPT_END: Symbol('</script>'),
    SCRIPT_LT: Symbol('script<'),
    SCRIPT_LT_SLASH: Symbol('script</'),
    SCRIPT_LT_SLASH_NAME: Symbol('script</name'),

    SLASH: Symbol('element/'),

    STARTTAG: Symbol('element'),
    STARTTAG_END: Symbol('element>'),

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

    map.set(XmlLexer.State.COMMENT, 'comment');
    map.set(XmlLexer.State.COMMENT_DASH, 'comment');
    map.set(XmlLexer.State.COMMENT_DASH_DASH, 'comment');
    map.set(XmlLexer.State.COMMENT_END, 'comment');
    map.set(XmlLexer.State.COMMENT_START, 'comment');

    map.set(XmlLexer.State.EMPTYTAG_END, 'keyword');

    map.set(XmlLexer.State.ENDTAG, 'keyword');
    map.set(XmlLexer.State.ENDTAG_NAME, 'html_element_name');
    map.set(XmlLexer.State.ENDTAG_OTHER, '');
    map.set(XmlLexer.State.ENDTAG_SPACE, '');

    map.set(XmlLexer.State.ENTITYREF, 'html_entity');

    map.set(XmlLexer.State.GT, 'keyword');

    map.set(XmlLexer.State.LT, 'keyword');
    map.set(XmlLexer.State.LT_BANG, '');
    map.set(XmlLexer.State.LT_BANG_DASH, '');
    map.set(XmlLexer.State.LT_QUESTION, 'keyword');

    map.set(XmlLexer.State.SCRIPT, '');
    map.set(XmlLexer.State.SCRIPT_END, '');
    map.set(XmlLexer.State.SCRIPT_LT, '');
    map.set(XmlLexer.State.SCRIPT_LT_SLASH, '');
    map.set(XmlLexer.State.SCRIPT_LT_SLASH_NAME, '');

    map.set(XmlLexer.State.SLASH, '');

    map.set(XmlLexer.State.STARTTAG, 'html_element_name');
    map.set(XmlLexer.State.STARTTAG_END, 'keyword');

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
        if (token.end - token.start === 1)
          return XmlLexer.State.AMPERSAND;
        break;
      case XmlLexer.State.LT_BANG:
        return XmlLexer.State.LT;
      case XmlLexer.State.LT_BANG_DASH:
        return XmlLexer.State.LT_BANG;
      case XmlLexer.State.SCRIPT_END:
      case XmlLexer.State.SCRIPT_LT_SLASH:
      case XmlLexer.State.SCRIPT_LT_SLASH_NAME:
        switch (token.end - token.start) {
          case 1:
            return XmlLexer.State.SCRIPT_LT;
          case 2:
            return XmlLexer.State.SCRIPT_LT_SLASH;
          default:
            return XmlLexer.State.SCRIPT_LT_SLASH_NAME;
        }
        break;
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
        if (charCode === Unicode.NUMBER_SIGN)
          this.restartToken(XmlLexer.State.CHARREF);
        else if (this.isNameStartChar(charCode))
          this.restartToken(XmlLexer.State.ENTITYREF);
        else
          this.state = Lexer.State.ZERO;
        return;

      case XmlLexer.State.CHARREF:
        if (charCode >= Unicode.DIGIT_ZERO && charCode <= Unicode.DIGIT_NINE)
          this.finishToken(XmlLexer.State.CHARREF_DIGIT);
        else if (charCode === Unicode.LATIN_SMALL_LETTER_X)
          this.finishToken(XmlLexer.State.CHARREF_XDIGIT);
        else
          this.endToken();
        return;

      case XmlLexer.State.ENTITYREF:
      case XmlLexer.State.CHARREF_DIGIT:
      case XmlLexer.State.CHARREF_XDIGIT:
        // Eat up to semicolon ";"
        if (charCode === Unicode.SEMICOLON || !this.isNameChar(charCode))
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
        if (charCode === Unicode.EQUALS_SIGN)
          this.finishToken(XmlLexer.State.ATTRNAME_EQ);
        else if (this.isNameChar(charCode))
          this.extendToken();
        else if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME_SPACE);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;
      case XmlLexer.State.ATTRNAME_EQ:
        // attrName '=' '\'' | attrName '=' '"' | attrName '=' space
        if (charCode === Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE1_START);
        else if (charCode === Unicode.QUOTATION_MARK)
          this.finishToken(XmlLexer.State.ATTRVALUE2_START);
        else if (this.isWhitespaceChar(charCode))
          this.extendToken();
        else
          processStartTag(this, charCode, XmlLexer.State.ATTRVALUE);
        return;
      case XmlLexer.State.ATTRNAME_SPACE:
        // attrName space '=' | attrName space attrName
        if (charCode === Unicode.EQUALS_SIGN)
          this.finishToken(XmlLexer.State.ATTRNAME_EQ);
        else if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME);
        else if (this.isWhitespaceChar(charCode))
          this.extendToken();
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTROTHER
      //
      case XmlLexer.State.ATTROTHER:
        if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // ATTRVALUE
      //
      case XmlLexer.State.ATTRVALUE:
        // attribute value without quote
        if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTRVALUE);
        return;

      case XmlLexer.State.ATTRVALUE1:
        // Eat up to single-quote "'". ">" terminates attribute value.
        if (charCode === Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else
          this.extendToken();
        return;

      case XmlLexer.State.ATTRVALUE1_START:
        if (charCode === Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else
          this.finishToken(XmlLexer.State.ATTRVALUE1);
        return;

      case XmlLexer.State.ATTRVALUE2:
        // Eat up to double-quote "\"". ">" terminates attribute value.
        if (charCode === Unicode.QUOTATION_MARK)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else
          this.extendToken();
        return;

      case XmlLexer.State.ATTRVALUE2_START:
        if (charCode === Unicode.APOSTROPHE)
          this.finishToken(XmlLexer.State.ATTRVALUE_END);
        else
          this.finishToken(XmlLexer.State.ATTRVALUE2);
        return;

      case XmlLexer.State.ATTRVALUE_END:
        if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.ATTRNAME);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      case XmlLexer.State.ATTRVALUE_SPACE:
        if (this.isWhitespaceChar(charCode))
          this.extendToken();
        else if (this.isNameStartChar(charCode))
           this.finishToken(XmlLexer.State.ATTRNAME);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
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
        if (charCode === Unicode.HYPHEN_MINUS)
          this.finishToken(XmlLexer.State.COMMENT_DASH);
        else
          this.extendToken();
        return;
      case XmlLexer.State.COMMENT_DASH:
        if (charCode === Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.COMMENT_DASH_DASH);
        else
          this.finishToken(XmlLexer.State.COMMENT);
          return;
      case XmlLexer.State.COMMENT_DASH_DASH:
        if (charCode === Unicode.GREATER_THAN_SIGN)
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
      // ENDTAG
      //
      case XmlLexer.State.ENDTAG:
        if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ENDTAG_SPACE);
        else if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.ENDTAG_NAME);
        else
          this.finishToken(XmlLexer.State.ENDTAG_OTHER);
        return;
      case XmlLexer.State.ENDTAG_NAME:
        if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (this.isNameChar(charCode))
          this.extendToken();
        else
          this.finishToken(XmlLexer.State.ENDTAG_OTHER);
        return;
      case XmlLexer.State.ENDTAG_OTHER:
        if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else
          this.extendToken();
        return;
      case XmlLexer.State.ENDTAG_SPACE:
        if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (this.isWhitespaceChar(charCode))
          this.extendToken();
        else if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.ENDTAG_NAME);
        else
          this.finishToken(XmlLexer.State.ENDTAG_OTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // LT
      //
      case XmlLexer.State.LT:
        if (charCode === Unicode.EXCLAMATION_MARK)
          this.restartToken(XmlLexer.State.LT_BANG);
        else if (charCode === Unicode.GREATER_THAN_SIGN)
          this.finishToken(XmlLexer.State.GT);
        else if (charCode === Unicode.SOLIDUS)
          this.restartToken(XmlLexer.State.ENDTAG);
        else if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.STARTTAG);
        else if (charCode === Unicode.QUESTION_MARK)
          this.restartToken(XmlLexer.State.LT_QUESTION);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_BANG:
        if (charCode === Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.LT_BANG_DASH);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_BANG_DASH:
        if (charCode === Unicode.HYPHEN_MINUS)
          this.restartToken(XmlLexer.State.COMMENT_START);
        else
          this.endToken();
        return;
      case XmlLexer.State.LT_QUESTION:
        if (this.isNameStartChar(charCode))
          this.finishToken(XmlLexer.State.STARTTAG);
        else
          this.endToken();
        return;

      ////////////////////////////////////////////////////////////
      //
      // SCRIPT
      //
      case XmlLexer.State.SCRIPT:
        if (charCode === Unicode.LESS_THAN_SIGN) {
          this.finishToken(XmlLexer.State.SCRIPT_LT);
          return;
        }
        feedCharToScriptLexer(this, charCode);
        this.extendToken();
        return;

      case XmlLexer.State.SCRIPT_END:
        processScriptEnd(this);
        return;

      case XmlLexer.State.SCRIPT_LT:
        if (charCode === Unicode.SOLIDUS) {
          this.restartToken(XmlLexer.State.SCRIPT_LT_SLASH);
          return;
        }

        feedCharToScriptLexer(this, Unicode.LESS_THAN_SIGN);
        if (charCode === Unicode.LESS_THAN_SIGN) {
          this.finishToken(XmlLexer.State.SCRIPT_LT);
          return;
        }
        feedCharToScriptLexer(this, charCode);
        this.finishToken(XmlLexer.State.SCRIPT);
        return;

      case XmlLexer.State.SCRIPT_LT_SLASH:
        if (this.isNameStartChar(charCode)) {
          this.restartToken(XmlLexer.State.SCRIPT_LT_SLASH_NAME);
          return;
        }
        feedStringToScriptLexer(this, '</');
        if (charCode === Unicode.LESS_THAN_SIGN) {
          this.finishToken(XmlLexer.State.SCRIPT_LT);
          return;
        }
        feedCharToScriptLexer(this, charCode);
        this.finishToken(XmlLexer.State.SCRIPT);
        return;

      case XmlLexer.State.SCRIPT_LT_SLASH_NAME:
        if (this.isNameChar(charCode)) {
          this.extendToken();
          return;
        }

        if (charCode === Unicode.GREATER_THAN_SIGN && isEndOfScript(this)) {
          this.restartToken(XmlLexer.State.SCRIPT_END);
          return;
        }

        {
          var document = this.range.document;
          var end = this.lastToken.end;
          for (var offset = this.lastToken.start; offset < end; ++offset) {
            feedCharToScriptLexer(this, document.charCodeAt_(offset));
          }
        }
        if (charCode === Unicode.LESS_THAN_SIGN) {
          this.finishToken(XmlLexer.State.SCRIPT_LT);
          return;
        }
        feedCharToScriptLexer(this, charCode);
        this.finishToken(XmlLexer.State.SCRIPT);
        return;

      ////////////////////////////////////////////////////////////
      //
      // SLASH
      //
      case XmlLexer.State.SLASH:
        if (charCode === Unicode.GREATER_THAN_SIGN)
          this.restartToken(XmlLexer.State.EMPTYTAG_END);
        else if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      ////////////////////////////////////////////////////////////
      //
      // STARTTAG
      //
      case XmlLexer.State.STARTTAG:
        if (this.isNameChar(charCode))
          this.extendToken();
        else if (this.isWhitespaceChar(charCode))
          this.finishToken(XmlLexer.State.ATTRVALUE_SPACE);
        else
          processStartTag(this, charCode, XmlLexer.State.ATTROTHER);
        return;

      case XmlLexer.State.STARTTAG_END:
        processStartTagEnd(this, charCode);
        return;

      ////////////////////////////////////////////////////////////
      //
      // TEXT
      //
      case XmlLexer.State.TEXT:
        if (charCode === Unicode.AMPERSAND)
          this.finishToken(XmlLexer.State.AMPERSAND);
        else if (charCode === Unicode.LESS_THAN_SIGN)
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
   */
  function feedCharToScriptLexer(lexer, charCode) {
    var tokenData = lexer.tokenData;
    if (!tokenData)
      return;
    var scriptLexer = tokenData.scriptLexer;
    var offset = scriptLexer.scanOffset;
    while (offset === scriptLexer.scanOffset) {
      scriptLexer.feedCharacter(charCode);
    }
  }

  /**
   * @param {!XmlLexer} lexer
   * @param {string} string
   */
  function feedStringToScriptLexer(lexer, string) {
    var tokenData = lexer.tokenData;
    if (!tokenData)
      return;
    var scriptLexer = tokenData.scriptLexer;
    for (var i = 0; i < string.length; ++i) {
      scriptLexer.feedCharacter(string.charCodeAt(i));
    }
  }

  /**
   * @param {!XmlLexer} lexer
   * @param {!Lexer.Token} token
   * @return {string}
   */
  function getTagNameFromToken(lexer, token) {
    var range = lexer.range;
    switch (token.state) {
      case XmlLexer.State.SCRIPT_LT_SLASH_NAME:
        range.collapseTo(token.start + 2);
        break;
      case XmlLexer.State.STARTTAG:
        range.collapseTo(token.start);
        break;
      default:
        console.log('getTagNameFromToken', token);
        console.assert(false);
    }
    range.end = token.end;
    return lexer.ignoreCase_ ? range.text.toLowerCase() : range.text;
  }

  /**
   * @param {!XmlLexer} lexer
   * @return {boolean}
   */
  function isEndOfScript(lexer) {
    var lastToken = /** @type {!Lexer.Token}*/ (lexer.lastToken);
    var tagName = getTagNameFromToken(lexer, lastToken);
    return tagName === lexer.tokenData.scriptTagName;
  }

  /**
   * @param {!XmlLexer} lexer
   */
  function processScriptEnd(lexer) {
    lexer.endToken();
    var tokenData = lexer.tokenData;
    if (!tokenData)
      return;
    var scriptLexer = tokenData.scriptLexer;
    lexer.tokenData = null;
    // Remove tokens after "</script>".
    scriptLexer.adjustScanOffset(lexer.lastToken.start);
  }

  /**
   * @param {!XmlLexer} lexer
   * @param {number} charCode
   * @param {!XmlLexer.State} defaultState
   */
  function processStartTag(lexer, charCode, defaultState) {
    if (charCode === Unicode.GREATER_THAN_SIGN)
      lexer.finishToken(XmlLexer.State.STARTTAG_END);
    else if (charCode === Unicode.QUESTION_MARK)
      lexer.finishToken(XmlLexer.State.SLASH);
    else if (charCode === Unicode.SOLIDUS)
      lexer.finishToken(XmlLexer.State.SLASH);
    else if (lexer.state === defaultState)
      lexer.extendToken();
    else
      lexer.finishToken(defaultState);
  }

  /**
   * @param {!XmlLexer} lexer
   * @param {number} charCode
   *
   * This function is called when |XmlLexer| gets a character after '<' in
   * start-tag. If start-tag name is "script", we pass a character
   * to script lexer unless a character is "<".
   */
  function processStartTagEnd(lexer, charCode) {
    lexer.endToken();
    if (!lexer.childLexerMap_.size)
      return;
    var it = lexer.tokens.find(/** @type {!Lexer.Token} */(lexer.lastToken));
    while (it.data.state !== XmlLexer.State.STARTTAG) {
      it = it.previous();
    }
    var token = it.data;
    var tagName = getTagNameFromToken(lexer, token);
    var childLexer = lexer.childLexerMap_.get(tagName);
    if (!childLexer)
      return;

    lexer.tokenData = {
      scriptLexer: childLexer,
      scriptTagName: tagName
    };

    // Handle script fragment after <script>
    if (charCode === Unicode.LESS_THAN_SIGN) {
      // Script fragment starts with "<".
      lexer.startToken(XmlLexer.State.SCRIPT_LT);
      return;
    }
    childLexer.scanOffset = lexer.scanOffset;
    lexer.startToken(XmlLexer.State.SCRIPT);
    feedCharToScriptLexer(lexer, charCode);
  }

  Object.defineProperties(XmlLexer.prototype, {
    // Properties
    childLexerMap_: {writable: true},
    ignoreCase_: {value: false, writable: true},

    // Methods
    didShrinkLastToken: {value: didShrinkLastToken},
    feedCharacter: {value: feedCharacter},
  });

  // TODO(eval1749): Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(XmlLexer, 'keywords', {
    get: function() { return xmlOptions.keywords; }
  });

  global['XmlLexer'] = XmlLexer;
})({
  keywords: Lexer.createKeywords([
      'xi:include',
      'xml:base',
      'xml:lang',
      'xmlns:',
      'xml:space'
])});
