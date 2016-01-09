// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.FindAndReplace = (function() {
  /**
   * @param {string} string
   * @param {!Range.Case} stringCase
   * @return string
   */
  function caseReplace(string, stringCase) {
    if (!string.length)
      return string;
    switch (stringCase) {
      case Range.Case.CAPITALIZED_TEXT: {
        for (let index = 0; index < string.length; ++index) {
          let charCode = string.charCodeAt(index);
          let ucd = Unicode.UCD[charCode];
          if (ucd.category === Unicode.Category.Lu ||
              ucd.category === Unicode.Category.Ll ||
              ucd.category === Unicode.Category.Lt) {
            return string.substr(0, index) +
                   string.substr(index, 1).toLocaleUpperCase() +
                   string.substr(index + 1);
          }
        }
        return string;
      }
      case Range.Case.CAPITALIZED_WORDS: {
        /** @type {string} */
        let newString = '';
        /** @type {boolean}*/
        let inWord = false;
        for (let index = 0; index < string.length; ++index) {
          /** @type {number} */
          const charCode = string.charCodeAt(index);
          const ucd = Unicode.UCD[charCode];
          /** @type {boolean} */
          const upperCase = ucd.category === Unicode.Category.Lu ||
                          ucd.category === Unicode.Category.Lt;
          /** @type {boolean} */
          const lowerCase = ucd.category === Unicode.Category.Ll;
          /** @type {number} */
          let newCharCode = charCode;
          if (inWord) {
            if (upperCase) {
              newCharCode = String.fromCharCode(charCode).toLocaleLowerCase()
                  .charCodeAt(0);
            } else if (!lowerCase) {
              inWord = false;
            }
          } else if (upperCase) {
            inWord = true;
          } else if (lowerCase) {
            newCharCode = String.fromCharCode(charCode).toLocaleUpperCase()
                .charCodeAt(0);
            inWord = true;
          }
          newString += String.fromCharCode(newCharCode);
        }
        return newString;
      }
      case Range.Case.LOWER:
        return string.toLocaleLowerCase()
      case Range.Case.LOWER:
        return string.toLocaleLowerCase()
      case Range.Case.MIXED:
        return string;
      case Range.Case.UPPER:
        return string.toLocaleUpperCase()
    }
    throw `Invalid Range.Case ${stringCase}`;
  }

  /**
   * @param {!Window} window
   * @param {string} searchText
   * @param {!FindAndReplaceOptions} findOptions
   * @return {Editor.RegExp}
   */
  function createRegExp(window, searchText, findOptions) {
    if (searchText === '')
      return null;
    try {
      return new Editor.RegExp(searchText, {
        backward: findOptions.backward,
        ignoreCase: findOptions.ignoreCase,
        matchExact: !findOptions.useRegExp,
        matchWord: findOptions.matchWholeWord,
        multiline: !findOptions.useRegExp,
      });
    } catch (error) {
      window.status = error.toString();
      throw error;
    }
  }

  /**
   * @param {!Window} window
   * @param {string} searchText
   * @param {!FindAndReplaceOptions} findOptions
   * @return {Editor.RegExp}
   */
  function find(window, searchText, findOptions) {
    /** @type {Editor.RegExp} */
    const regexp = createRegExp(window, searchText, findOptions);
    if (!regexp)
      return null;

    /** @type{!TextDocument} */
    const document = window.document;
    /** @type{!Range} */
    const range = window.selection.range;

    /**
     * @param {Array.<!Editor.RegExp.Match>} matches
     */
    function finish(matches) {
      if (!matches) {
        window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                            {text: regexp.source});
        return regexp;
      }
      /** @type {!Editor.RegExp.Match} */
      const match = matches[0];
      if (match.start === range.start && match.end === range.end) {
        window.status = Strings.IDS_PASSED_END;
        return regexp;
      }
      range.collapseTo(match.start);
      range.end = match.end;
      window.selection.startIsActive = regexp.backward;
      window.status = Editor.localizeText(Strings.IDS_FIND_FOUND,
                                          {text: regexp.source});
      return regexp;
    }

    if (shouldFindInSelection(findOptions, range)) {
     /** @type {Array.<!Editor.RegExp.Match>} */
      const matches = document.match_(regexp, range.start, range.end);
      if (!matches)
        return finish(matches);
      /** @type {!Editor.RegExp.Match} */
      const match = matches[0];
      if (match.start !== range.start || match.end !== range.end)
        return finish(matches);
    }

    /** @type {number} */
    const end = document.length;
    if (regexp.backward) {
     /** @type {Array.<!Editor.RegExp.Match>} */
      const matches = document.match_(regexp, 0, range.start);
      if (matches)
        return finish(matches);
      return finish(document.match_(regexp, range.start, end));
    }

     /** @type {Array.<!Editor.RegExp.Match>} */
    const matches = document.match_(regexp, range.end, end);
    if (matches)
      return finish(matches);
    return finish(document.match_(regexp, 0, range.end));
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function hasUpperCase(text) {
    for (let i = 0; i < text.length; ++i) {
      const data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category === Unicode.Category.Lu ||
          data.category === Unicode.Category.Lt) {
        return true;
      }
    }
    return false;
  }

  /** @enum {!symbol} */
  const State = {
    BACKSLASH: Symbol('BACKSLASH'),
    BACKSLASH_C: Symbol('BACKSLASH_C'), // \cC = control character
    BACKSLASH_DIGIT: Symbol('BACKSLASH_DIGIT'), // \ooo = octal
    BACKSLASH_U: Symbol('BACKSLASH_U'), // \uUUUU = unicode code point
    BACKSLASH_X: Symbol('BACKSLASH_X'), // \xXX = hexadecimal
    DOLLAR: Symbol('DOLLAR'),
    DOLLAR_BRACKET: Symbol('DOLLAR_BRACKET'), // ${name}
    DOLLAR_DIGIT: Symbol('DOLLAR_DIGIT'), // $<digit>+
    START: Symbol('START')
  };

  /**
   * @param {!TextDocument} document
   * @param {string} newSource
   * @param {!Array.<Editor.RegExp.Match>} matches
   * @return string
   */
  function parseReplacement(document, newSource, matches) {
    function parseHexDigit(charCode) {
      if (charCode >= Unicode.DIGIT_ZERO &&
          charCode <= Unicode.DIGIT_NINE) {
        return charCode - Unicode.DIGIT_ZERO;
      }
      if (charCode >= Unicode.LATIN_CAPITAL_LETTER_A &&
          charCode >= Unicode.LATIN_CAPITAL_LETTER_F) {
        return charCode - Unicode.LATIN_CAPITAL_LETTER_A + 10;
      }
      if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
          charCode >= Unicode.LATIN_SMALL_LETTER_F) {
        return charCode - Unicode.LATIN_SMALL_LETTER_A + 10;
      }
      return -1;
    }

    /** @type {string} */
    let newText = '';

    function addChar(charCode) {
      newText += String.fromCharCode(charCode);
    }

    function addMatch(nth) {
      let match = matches[nth];
      if (!match)
        return;
      newText += document.slice(match.start, match.end);
    }

    function addNamedMatch(name) {
      const match = matches.find(function(match) {
        return match.name === name;
      });
      if (!match)
        return;
      newText += document.slice(match.start, match.end);
    }

    /** @type {number} */
    let accumulator = 0;
    /** @type {string} */
    let matchName = '';
    /** @type {number} */
    let digitCount = 0;
    /** @type {!State} */
    let state = State.START;
    for (let index = 0; index < newSource.length; ++index) {
      let charCode = newSource.charCodeAt(index);
      switch (state) {
        case State.START:
          switch (charCode) {
            case Unicode.DOLLAR_SIGN:
              state = State.DOLLAR;
              break;
            case Unicode.REVERSE_SOLIDUS:
              state = State.BACKSLASH;
              break;
            default:
              addChar(charCode);
              break;
          }
          break;
        case State.BACKSLASH:
          state = State.START;
          switch (charCode) {
            case Unicode.DIGIT_ZERO: case Unicode.DIGIT_ONE:
            case Unicode.DIGIT_TWO: case Unicode.DIGIT_THREE:
            case Unicode.DIGIT_FOUR: case Unicode.DIGIT_FIVE:
            case Unicode.DIGIT_SIX: case Unicode.DIGIT_SEVEN:
              accumulator = charCode - Unicode.DIGIT_ZERO;
              digitCount = 1;
              state = State.BACKSLASH_DIGIT;
              break;
            case Unicode.LATIN_SMALL_LETTER_A:
              newText += '\a';
              break;
            case Unicode.LATIN_SMALL_LETTER_B:
              newText += '\b';
              break;
            case Unicode.LATIN_SMALL_LETTER_C:
              state = State.BACKSLASH_C;
              break;
            case Unicode.LATIN_SMALL_LETTER_E:
              newText += '\x1B';
              break;
            case Unicode.LATIN_SMALL_LETTER_F:
              newText += '\f';
              break;
            case Unicode.LATIN_SMALL_LETTER_N:
              newText += '\n';
              break;
            case Unicode.LATIN_SMALL_LETTER_R:
              newText += '\r';
              break;
            case Unicode.LATIN_SMALL_LETTER_T:
              newText += '\t';
              break;
            case Unicode.LATIN_SMALL_LETTER_U:
              state = State.BACKSLASH_U;
              break;
            case Unicode.LATIN_SMALL_LETTER_V:
              newText += '\v';
              break;
            case Unicode.REVERSE_SOLIDUS:
              newText += '\\';
              break;
            default:
              // Insert a character instead of throwing an exception.
              addChar(charCode);
              break;
          }
          break;
        case State.BACKSLASH_DIGIT:
          if (charCode >= Unicode.DIGIT_ZERO &&
              charCode <= Unicode.DIGIT_SEVEN) {
            accumulator *= 8;
            accumulator += charCode - Unicode.DIGIT_ZERO;
            ++digitCount;
          } else {
            digitCount = 3;
            --index;
          }

          if (digitCount === 3) {
            addChar(accumulator);
            state = State.START;
          }
          break;
        case State.BACKSLASH_C:
          if (charCode >= Unicode.COMMERCIAL_AT &&
              charCode <= Unicode.LOW_LINE) {
            addChar(charCode - Unicode.COMMERCIAL_AT);
          } else if (charCode >= Unicode.LATIN_SMALL_LETTER_A &&
                     charCode >= Unicode.LATIN_SMALL_LETTER_Z) {
            addChar(charCode - Unicode.LATIN_SMALL_LETTER_A + 1);
          } else {
            // ignore invalid \cX
          }
          state = State.START;
          break;
        case State.BACKSLASH_U: {
          /** @type {number} */
          let digit = parseHexDigit(charCode);
          if (digit >= 0) {
            ++digitCount;
            accumulator *= 16;
            accumulator += digit;
            if (digitCount === 4) {
              addChar(accumulator);
              digitCount = 0;
              accumulator = 0;
              state = State.START;
            }
            break;
          }
          // ignore invalid \uUUUU
          state = State.START;
          break;
        }
        case State.BACKSLASH_X: {
          /** @type {number} */
          let digit = parseHexDigit(charCode);
          if (digit >= 0) {
            ++digitCount;
            accumulator *= 16;
            accumulator += digit;
            if (digitCount === 2) {
              addChar(accumulator);
              digitCount = 0;
              accumulator = 0;
              state = State.START;
            }
            break;
          }
          // ignore invalid \xXX
          state = State.START;
          break;
        }
        case State.DOLLAR:
          if (charCode >= Unicode.DIGIT_ZERO &&
              charCode <= Unicode.DIGIT_NINE) {
            accumulator = charCode - Unicode.DIGIT_ZERO;
            state = State.DOLLAR_DIGIT;
          } else if (charCode === Unicode.AMPERSAND) {
            addMatch(0);
            state = State.START;
          } else if (charCode === Unicode.LEFT_CURLY_BRACKET) {
            state = State.DOLLAR_BRACKET;
          } else {
            addChar(Unicode.DOLLAR_SIGN);
            addChar(charCode);
            state = State.START;
          }
          break;
        case State.DOLLAR_DIGIT:
          if (charCode >= Unicode.DIGIT_ZERO &&
              charCode <= Unicode.DIGIT_NINE) {
            accumulator *= 10;
            accumulator += charCode - Unicode.DIGIT_ZERO;
            break;
          }
          addMatch(accumulator);
          accumulator = 0;
          if (charCode === Unicode.REVERSE_SOLIDUS) {
            state = State.BACKSLASH;
            break;
          }
          addChar(charCode);
          state = State.START;
          break;
        case State.DOLLAR_BRACKET:
          if (charCode === Unicode.RIGHT_CURLY_BRACKET) {
            addNamedMatch(matchName);
            matchName = '';
            state = State.START;
            break;
          }
          matchName += String.fromCharCode(charCode);
          break;
        default:
          throw `Unknown state ${state}`;
      }
    }
    if (state === State.DOLLAR_DIGIT)
      addMatch(accumulator);
    return newText;
  }

  /**
   * @param {!Window} window
   * @param {string} searchText
   * @param {string} replaceText
   * @param {!FindAndReplaceOptions} findOptions
   * @return {Editor.RegExp}
   */
  function replaceOne(window, searchText, replaceText, findOptions) {
    /** @type {Editor.RegExp} */
    const regexp = createRegExp(window, searchText, findOptions);
    if (!regexp)
      return null;
    const selection = window.selection;
    const range = selection.range;
    const document = range.document;
    const matches = shouldFindInSelection(findOptions, range) ?
        document.match_(regexp, range.start, range.end) :
        document.match_(regexp, 0, document.length);
    if (!matches) {
      window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                          {text: regexp.source});
      return regexp;
    }
    range.collapseTo(matches[0].start);
    range.end = matches[0].end;
    /** @type {boolean} */
    const casePreserve = shouldPreserveCase(findOptions, replaceText);
    if (!regexp.matchExact)
      replaceText = parseReplacement(document, replaceText, matches);
    if (casePreserve)
      replaceText = caseReplace(replaceText, range.analyzeCase());
    range.text = replaceText;
    selection.startIsActive = false;
    window.status = Editor.localizeText(Strings.IDS_REPLACED,
                                        {count: 1, text: regexp.source});
    window.makeSelectionVisible();
    return regexp;
  }

  /**
   * @param {!Window} window
   * @param {string} searchText
   * @param {string} replaceText
   * @param {!FindAndReplaceOptions} findOptions
   * @return {Editor.RegExp}
   */
  function replaceAll(window, searchText, replaceText, findOptions) {
    /** @type {Editor.RegExp} */
    let regexp = createRegExp(window, searchText, findOptions);
    if (!regexp)
      return null;
    const document = window.document;
    const selection = window.selection;
    const selection_range = selection.range;
    const replaceRange = new Range(selection_range);
    if (!shouldFindInSelection(findOptions, replaceRange)) {
      replaceRange.start = 0;
      replaceRange.end = document.length;
    }
    /** @type {boolean} */
    const casePreserve = shouldPreserveCase(findOptions, replaceText);
    /** @type {!Range} */
    const range = new Range(document);
    /** @type {number} */
    let replacedCount = 0;
    document.undoGroup('replace all', function() {
      while (!replaceRange.collapsed) {
        const matches = document.match_(regexp, replaceRange.start,
                                        replaceRange.end);
        if (!matches)
          break;
        ++replacedCount;
        range.collapseTo(matches[0].start);
        range.end = matches[0].end;
        let newText = replaceText;
        if (!regexp.matchExact)
          newText = parseReplacement(document, newText, matches);
        if (casePreserve)
          newText = caseReplace(newText, range.analyzeCase());
        range.text = newText;
        replaceRange.start = range.end;
      }
    });
    if (replacedCount) {
      selection.startIsActive = false;
      window.status = Editor.localizeText(Strings.IDS_REPLACED,
                                          {count: replacedCount,
                                           text: regexp.source});
      window.makeSelectionVisible();
    } else {
      window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                          {text: regexp.source});
    }
    return regexp;
  }

  /**
   * @param {!FindAndReplaceOptions} findOptions
   * @param {!Range} range
   * @return {boolean}
   */
  function shouldFindInSelection(findOptions, range) {
    if (range.collapsed)
      return false;
    /** @type {!TextDocument} */
    const document = range.document;
    for (let offset = range.start; offset < range.end; ++offset) {
      if (document.charCodeAt(offset) === Unicode.LF)
        return true;
    }
    return false;
  }

  /**
   * @param {!FindAndReplaceOptions} findOptions
   * @param {string} text
   * @return {boolean}
   */
  function shouldPreserveCase(findOptions, text) {
    return !hasUpperCase(text) && findOptions.casePreserveReplacement;
  }

  let FindAndReplace = Object.defineProperties({}, {
    find: {value: find},
    replaceOne: {value: replaceOne},
    replaceAll: {value: replaceAll}
  });
  Object.freeze(FindAndReplace);
  return FindAndReplace;
})();
