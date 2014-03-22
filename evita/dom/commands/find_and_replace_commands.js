// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @enum {number} */
  var ControlId = {
    WHAT: 1001,
    WITH: 1002,
    WORD: 1003,
    CASE: 1004,
    REGEX: 1005,
    ALLDOCS: 1006,
    PRESERVE: 1007,
    UP: 1009,
    DOWN: 1010,
    SELECTION: 1012,
    WHOLE_FILE: 1013,
    NEXT: 1014,
    PREVIOUS: 1015,
    REPLACE: 1016,
    REPLACE_ALL: 1017,
    EXACT: 1018
  };

  /** @type {?Form} */
  var form = null;

  /**
   * @param {string} string
   * @param {!Range.Case} string_case
   * @return string
   */
  function caseReplace(string, string_case) {
    if (!string.length)
      return string;
    switch (string_case) {
      case Range.Case.CAPITALIZED_TEXT: {
        for (var index = 0; index < string.length; ++index) {
          var char_code = string.charCodeAt(index);
          var ucd = Unicode.UCD[char_code];
          if (ucd.category == Unicode.Category.Lu ||
              ucd.category == Unicode.Category.Ll ||
              ucd.category == Unicode.Category.Lt) {
            return string.substr(0, index) +
                   string.substr(index, 1).toLocaleUpperCase() +
                   string.substr(index + 1);
          }
        }
        return string;
      }
      case Range.Case.CAPITALIZED_WORDS: {
        var new_string = '';
        var in_word = false;
        for (var index = 0; index < string.length; ++index) {
          var char_code = string.charCodeAt(index);
          var ucd = Unicode.UCD[char_code];
          var upper_case = ucd.category == Unicode.Category.Lu ||
                           ucd.category == Unicode.Category.Lt;
          var lower_case = ucd.category == Unicode.Category.Ll;
          var new_char_code = char_code;
          if (in_word) {
            if (upper_case) {
              new_char_code = String.fromCharCode(char_code).toLocaleLowerCase()
                  .charCodeAt(0);
            } else if (!lower_case) {
              in_word = false;
            }
          } else if (upper_case) {
            in_word = true;
          } else if (lower_case) {
            new_char_code = String.fromCharCode(char_code).toLocaleUpperCase()
                .charCodeAt(0);
            in_word = true;
          }
          new_string += String.fromCharCode(new_char_code);
        }
        return new_string;
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
    throw 'Invalid Range.Case ' + string_case;
  }

  /**
   * @param {!Window} window
   * @param {!Form} form
   * @param {number} button_id
   * @return {?Editor.RegExp}
   */
  function createRegExp(window, form, button_id) {
    var search_text = form.get(ControlId.WHAT).value;
    if (search_text == '')
      return null;
    var backward = button_id == ControlId.PREVIOUS;
    try {
      return new Editor.RegExp(search_text, {
        backward: backward,
        ignoreCase: shouldIgnoreCase(form, search_text),
        matchExact: form.get(ControlId.EXACT).checked,
        matchWord: form.get(ControlId.WORD).checked,
        multiline: form.get(ControlId.REGEX).checked
      });
    } catch (error) {
      window.status = error.toString();
      throw error
    }
  }

  /**
   * @param {!Window} window
   * @param {!Form} form
   * @param {number} button_id
   * @return {?Editor.RegExp}
   */
  function doFind(window, form, button_id) {
    var regexp = createRegExp(window, form, button_id);
    if (!regexp)
      return null;
    var document = window.document;
    var range = window.selection.range;

    function finish(matches) {
      if (!matches) {
        window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                            {text: regexp.source});
        return regexp;
      }
      var match = matches[0];
      if (match.start == range.start && match.end == range.end) {
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

    if (shouldFindInSelection(form, range))
      return finish(range.match_(regexp));

    var end = document.length;
    if (regexp.backward) {
      var matches = document.match_(regexp, 0, range.start);
      if (matches)
        return finish(matches);
      return finish(document.match_(regexp, range.start, end));
    }

    var matches = document.match_(regexp, range.end, end);
    if (matches)
      return finish(matches);
    return finish(document.match_(regexp, 0, range.end));
  }

  /**
   * @param {!Window} window
   * @param {!Form} form
   */
  function doReplaceOne(window, form) {
    var regexp = createRegExp(window, form, ControlId.NEXT);
    if (!regexp)
      return;
    var selection = window.selection;
    var range = selection.range;
    var document = range.document;
    var matches = shouldFindInSelection(form, range) ?
        document.match_(regexp, range.start, range.end) :
        document.match_(regexp, 0, document.length);
    if (!matches) {
      window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                          {text: regexp.source});
      return regexp;
    }
    range.collapseTo(matches[0].start);
    range.end = matches[0].end;
    var new_text = form.get(ControlId.WITH).value;
    var case_preserve = shouldPreserveCase(form, new_text);
    if (!regexp.matchExact)
      new_text = parseReplacement(document, new_text, matches);
    if (case_preserve)
      new_text = caseReplace(new_text, range.analyzeCase());
    range.text = new_text;
    selection.startIsActive = false;
    window.status = Editor.localizeText(Strings.IDS_REPLACED,
                                        {count: 1, text: regexp.source});
    window.makeSelectionVisible();
    return regexp;
  }

  /**
   * @param {!Window} window
   * @param {!Form} form
   */
  function doReplaceAll(window, form) {
    var regexp = createRegExp(window, form, ControlId.NEXT);
    if (!regexp)
      return;
    var document = window.document;
    var selection = window.selection;
    var selection_range = selection.range;
    var replace_range = new Range(selection_range);
    if (!shouldFindInSelection(form, replace_range)) {
      replace_range.start = 0;
      replace_range.end = document.length;
    }
    var replace_text = form.get(ControlId.WITH).value;
    var case_preserve = shouldPreserveCase(form, replace_text);
    var num_replaced = 0;
    var range = new Range(document);
    document.undoGroup('replace all', function() {
      while (!replace_range.collapsed) {
        var matches = document.match_(regexp, replace_range.start,
                                      replace_range.end);
        if (!matches)
          break;
        ++num_replaced;
        range.collapseTo(matches[0].start);
        range.end = matches[0].end;
        var new_text = replace_text;
        if (!regexp.matchExact)
          new_text = parseReplacement(document, new_text, matches);
        if (case_preserve)
          new_text = caseReplace(new_text, range.analyzeCase());
        range.text = new_text;
        replace_range.start = range.end;
      }
    });
    if (num_replaced) {
      selection.startIsActive = false;
      window.status = Editor.localizeText(Strings.IDS_REPLACED,
                                          {count: num_replaced,
                                           text: regexp.source});
      window.makeSelectionVisible();
    } else {
      window.status = Editor.localizeText(Strings.IDS_FIND_NOT_FOUND,
                                          {text: regexp.source});
    }
    return regexp;
  }

  /** @return {!Form} */
  function ensureForm() {
    if (form)
      return form;
    form = new Form('FindDialogBox');
    // TODO(yosi) |global.findForm| is only for debugging purpose.
    global.findForm = form;

    function addAsDisabled(control) {
      form.add(control);
      control.disabled = true;
    }

    form.add(new TextFieldControl(ControlId.WHAT));
    addAsDisabled(new TextFieldControl(ControlId.WITH));

    addAsDisabled(new CheckboxControl(ControlId.ALLDOCS));
    addAsDisabled(new CheckboxControl(ControlId.CASE));
    addAsDisabled(new CheckboxControl(ControlId.PRESERVE));

    addAsDisabled(new RadioButtonControl('use', ControlId.EXACT));
    addAsDisabled(new RadioButtonControl('use', ControlId.WORD));
    addAsDisabled(new RadioButtonControl('use', ControlId.REGEX));

    addAsDisabled(new RadioButtonControl('direction', ControlId.UP));
    addAsDisabled(new RadioButtonControl('direction', ControlId.DOWN));

    addAsDisabled(new RadioButtonControl('where', ControlId.WHOLE_FILE));
    addAsDisabled(new RadioButtonControl('where', ControlId.SELECTION));

    addAsDisabled(new ButtonControl(ControlId.NEXT));
    addAsDisabled(new ButtonControl(ControlId.PREVIOUS));
    addAsDisabled(new ButtonControl(ControlId.REPLACE));
    addAsDisabled(new ButtonControl(ControlId.REPLACE_ALL));

    form.get(ControlId.EXACT).checked = true;
    form.get(ControlId.PRESERVE).checked = true;
    form.get(ControlId.UP).checked = true;
    form.get(ControlId.WHOLE_FILE).checked = true;

    form.get(ControlId.WHAT).addEventListener('change', function() {
      var can_find = form.get(ControlId.WHAT).value != '';

      form.get(ControlId.CASE).disabled = !can_find;

      // Find buttons
      form.get(ControlId.NEXT).disabled = !can_find;
      form.get(ControlId.PREVIOUS).disabled = !can_find;

      // Replace
      form.get(ControlId.WITH).disabled = !can_find;
      form.get(ControlId.REPLACE).disabled = !can_find;
      form.get(ControlId.REPLACE_ALL).disabled = !can_find;

      // Use
      form.get(ControlId.EXACT).disabled = !can_find;
      form.get(ControlId.WORD).disabled = !can_find;
      form.get(ControlId.REGEX).disabled = !can_find;
    });

    form.get(ControlId.WITH).addEventListener('change', function() {
      var can_replace = form.get(ControlId.WITH).value != '';
      form.get(ControlId.PRESERVE).disabled = !can_replace;

      // TODO(yosi) We should update find dialog box when selection is changed.
      // Replace in
      form.get(ControlId.SELECTION).disabled = !can_replace;
      form.get(ControlId.WHOLE_FILE).disabled = !can_replace;
    });

    form.get(ControlId.NEXT).addEventListener('click', function() {
      if (!Window.focus)
        return;
      doFind(/**@type{!Window}*/(Window.focus), /**@type{!Form}*/(form),
             ControlId.NEXT);
    });

    form.get(ControlId.PREVIOUS).addEventListener('click', function() {
      if (!Window.focus)
        return;
      doFind(/**@type{!Window}*/(Window.focus), /**@type{!Form}*/(form),
             ControlId.PREVIOUS);
    });

    form.get(ControlId.REPLACE).addEventListener('click', function() {
      if (!Window.focus)
        return;
      doReplaceOne(/**@type{!Window}*/(Window.focus), /**@type{!Form}*/(form));
    });

    form.get(ControlId.REPLACE_ALL).addEventListener('click', function() {
      if (!Window.focus)
        return;
      doReplaceAll(/**@type{!Window}*/(Window.focus), /**@type{!Form}*/(form));
    });

    form.realize();
    return form;
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function hasUpperCase(text) {
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Lt) {
        return true;
      }
    }
    return false;
  }

  /**
   * @param {!Document} document
   * @param {string} new_source
   * @param {!Array.<Editor.RegExp.Match>} matches
   * @return string
   */
  function parseReplacement(document, new_source, matches) {
    /** @enum {string} */
    var State = {
      BACKSLASH: 'BACKSLASH',
      BACKSLASH_C: 'BACKSLASH_C', // \cC = control character
      BACKSLASH_DIGIT: 'BACKSLASH_DIGIT', // \ooo = octal
      BACKSLASH_U: 'BACKSLASH_U', // \uUUUU = unicode code point
      BACKSLASH_X: 'BACKSLASH_X', // \xXX = hexadecimal
      DOLLAR: 'DOLLAR',
      DOLLAR_BRACKET: 'DOLLAR_BRACKET', // ${name}
      DOLLAR_DIGIT: 'DOLLAR_DIGIT', // $<digit>+
      START: 'START',
    };

    function parseHexDigit(char_code) {
      if (char_code >= Unicode.DIGIT_ZERO &&
          char_code <= Unicode.DIGIT_NINE) {
        return char_code - Unicode.DIGIT_ZERO;
      }
      if (char_code >= Unicode.LATIN_CAPITAL_LETTER_A &&
          char_code >= Unicode.LATIN_CAPITAL_LETTER_F) {
        return char_code - Unicode.LATIN_CAPITAL_LETTER_A + 10;
      }
      if (char_code >= Unicode.LATIN_SMALL_LETTER_A &&
          char_code >= Unicode.LATIN_SMALL_LETTER_F) {
        return char_code - Unicode.LATIN_SMALL_LETTER_A + 10;
      }
      return -1;
    }

    var new_text = '';

    function addChar(char_code) {
      new_text += String.fromCharCode(char_code);
    }

    function addMatch(nth) {
      var match = matches[nth];
      if (!match)
        return;
      new_text += document.slice(match.start, match.end);
    }

    function addNamedMatch(name) {
      var match = matches.find(function(match) {
        return match.name == name;
      });
      if (!match)
        return;
      new_text += document.slice(match.start, match.end);
    }

    var accumulatr = 0;
    var match_name = '';
    var num_digits = 0;
    var state = State.START;
    for (var index = 0; index < new_source.length; ++index) {
      var char_code = new_source.charCodeAt(index);
      switch (state) {
        case State.START:
          switch (char_code) {
            case Unicode.DOLLAR_SIGN:
              state = State.DOLLAR;
              break;
            case Unicode.REVERSE_SOLIDUS:
              state = State.BACKSLASH;
              break;
            default:
              addChar(char_code);
              break;
          }
          break;
        case State.BACKSLASH:
          state = State.START;
          switch (char_code) {
            case Unicode.DIGIT_ZERO: case Unicode.DIGIT_ONE:
            case Unicode.DIGIT_TWO: case Unicode.DIGIT_THREE:
            case Unicode.DIGIT_FOUR: case Unicode.DIGIT_FIVE:
            case Unicode.DIGIT_SIX: case Unicode.DIGIT_SEVEN:
              accumulatr = char_code - Unicode.DIGIT_ZERO;
              num_digits = 1;
              state = State.BACKSLASH_DIGIT;
              break;
            case Unicode.LATIN_SMALL_LETTER_A:
              new_text += '\a';
              break;
            case Unicode.LATIN_SMALL_LETTER_B:
              new_text += '\b';
              break;
            case Unicode.LATIN_SMALL_LETTER_C:
              state = State.BACKSLASH_C;
              break;
            case Unicode.LATIN_SMALL_LETTER_E:
              new_text += '\x1B';
              break;
            case Unicode.LATIN_SMALL_LETTER_F:
              new_text += '\f';
              break;
            case Unicode.LATIN_SMALL_LETTER_N:
              new_text += '\n';
              break;
            case Unicode.LATIN_SMALL_LETTER_R:
              new_text += '\r';
              break;
            case Unicode.LATIN_SMALL_LETTER_T:
              new_text += '\t';
              break;
            case Unicode.LATIN_SMALL_LETTER_U:
              state = State.BACKSLASH_U;
              break;
            case Unicode.LATIN_SMALL_LETTER_V:
              new_text += '\v';
              break;
            case Unicode.REVERSE_SOLIDUS:
              new_text += '\\';
              break;
            default:
              // Insert a character instead of throwing an exception.
              addChar(char_code);
              break;
          }
          break;
        case State.BACKSLASH_DIGIT:
          if (char_code >= Unicode.DIGIT_ZERO &&
              char_code <= Unicode.DIGIT_SEVEN) {
            accumulatr *= 8;
            accumulatr += char_code - Unicode.DIGIT_ZERO;
            ++num_digits;
          } else {
            num_digits = 3;
            --index;
          }

          if (num_digits == 3) {
            addChar(accumulatr);
            state = State.START;
          }
          break;
        case State.BACKSLASH_C:
          if (char_code >= Unicode.COMMERCIAL_AT &&
              char_code <= Unicode.LOW_LINE) {
            addChar(char_code - Unicode.COMMERCIAL_AT);
          } else if (char_code >= Unicode.LATIN_SMALL_LETTER_A &&
                     char_code >= Unicode.LATIN_SMALL_LETTER_Z) {
            addChar(char_code - Unicode.LATIN_SMALL_LETTER_A + 1);
          } else {
            // ignore invalid \cX
          }
          state = State.START;
          break;
        case State.BACKSLASH_U: {
          var digit = parseHexDigit(char_code);
          if (digit >= 0) {
            ++num_digits;
            accumulatr *= 16;
            accumulatr += digit;
            if (num_digits == 4) {
              addChar(accumulatr);
              num_digits = 0;
              accumulatr = 0;
              state = State.START;
            }
            break;
          }
          // ignore invalid \uUUUU
          state = State.START;
          break;
        }
        case State.BACKSLASH_X: {
          var digit = parseHexDigit(char_code);
          if (digit >= 0) {
            ++num_digits;
            accumulatr *= 16;
            accumulatr += digit;
            if (num_digits == 2) {
              addChar(accumulatr);
              num_digits = 0;
              accumulatr = 0;
              state = State.START;
            }
            break;
          }
          // ignore invalid \xXX
          state = State.START;
          break;
        }
        case State.DOLLAR:
          if (char_code >= Unicode.DIGIT_ZERO &&
              char_code <= Unicode.DIGIT_NINE) {
            accumulatr = char_code - Unicode.DIGIT_ZERO;
            state = State.DOLLAR_DIGIT;
          } else if (char_code == Unicode.AMPERSAND) {
            addMatch(0);
            state = State.START;
          } else if (char_code == Unicode.LEFT_CURLY_BRACKET) {
            state = State.DOLLAR_BRACKET;
          } else {
            addChar(Unicode.DOLLAR_SIGN);
            addChar(char_code);
            state = State.START;
          }
          break;
        case State.DOLLAR_DIGIT:
          if (char_code >= Unicode.DIGIT_ZERO &&
              char_code <= Unicode.DIGIT_NINE) {
            accumulatr *= 10;
            accumulatr += char_code - Unicode.DIGIT_ZERO;
            break;
          }
          addMatch(accumulatr);
          addChar(char_code);
          accumulatr = 0;
          state = State.START;
          break;
        case State.DOLLAR_BRACKET:
          if (char_code == Unicode.RIGHT_CURLY_BRACKET) {
            addNamedMatch(match_name);
            match_name = '';
            state = State.START;
            break;
          }
          match_name += String.fromCharCode(char_code);
          break;
        default:
          throw 'Unknown state ' + state;
      }
    }
    if (state == State.DOLLAR_DIGIT)
      addMatch(accumulatr);
    return new_text;
  }

  function showFindFormCommand() {
    ensureForm().show();
  }

  /**
   * @param {!Form} form
   * @param {!Range} range
   * @return {boolean}
   */
  function shouldFindInSelection(form, range) {
    if (range.collapsed)
      return false;
    var document = range.document;
    for (var offset = range.start; offset < range.end; ++offset) {
      if (document.charCodeAt_(offset) == Unicode.LF)
        return true;
    }
    return form.get(ControlId.SELECTION).checked;
  }

  /**
   * @param {!Form} form
   * @param {string} text
   */
  function shouldIgnoreCase(form, text) {
    if (hasUpperCase(text))
      return false;
    return !form.get(ControlId.CASE).checked;
  }

  /**
   * @param {!Form} form
   * @param {string} text
   */
  function shouldPreserveCase(form, text) {
    if (hasUpperCase(text))
      return false;
    return form.get(ControlId.PRESERVE).checked;
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindFormCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindFormCommand);

  Editor.bindKey(TextWindow, 'F3', function() {
    doFind(this, ensureForm(), ControlId.NEXT);
  });

  Editor.bindKey(TextWindow, 'Shift+F3', function() {
    doFind(this, ensureForm(), ControlId.PREVIOUS);
  });
})();
