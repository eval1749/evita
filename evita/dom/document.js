// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /**
   * @param {string} absolute_filename
   * @return {Document}
   */
  function findDocumentOnFile(absolute_filename) {
    var canonical_filename = absolute_filename.toLocaleLowerCase();
    return /** @type{Document} */ (Document.list.find(
        function(document) {
          return document.filename.toLocaleLowerCase() == canonical_filename;
        }));
  }

  /** @const @type {Map.<string, string>} */
  var WORD_CLASS_MAP = (function() {
    /** @param {string} name */
    function wordClass(name) {
      var word_class = name.charAt(0);
      if (word_class == 'L' || word_class == 'N')
        return 'w';
      return word_class;
    }
    var map = new Map();
    Unicode.CATEGORY_SHORT_NAMES.forEach(function(name) {
      map.set(name, wordClass(name));
    });
    return map;
  })();

  /** @enum{string} */
  var WordClass = {
    BLANK: 'Z',
    WORD: 'w'
  };

  function throwInvalidUnit(unit) {
    throw 'Invalid unit: ' + unit;
  }

  function throwNYI(name, unit) {
    throw 'NYI Range.prototype.' + name + ' Unit.' + unit;
  }

  function throwUnsupportedUnit(name, unit) {
    throw 'Range.prototype.' + name + ' does not support Unit.' + unit;
  }

  /**
   * @param {number} char_code
   * @return {WordClass|null}
   */
  function wordClassOf(char_code) {
    return WORD_CLASS_MAP.get(Unicode.UCD[char_code].category);
  }

  /**
   * @param {!Document} document
   * @param {number} position
   * @return {WordClass|null}
   */
  function wordClassAt(document, position) {
    return wordClassOf(document.charCodeAt_(position));
  }

  /** @enum{number} */
  global.Document.Obsolete = {
    NO: 0,
    CHECKING: 1,
    IGNORE: 2,
    UNKNOWN: 3,
    YES: 4
  };

  /** @type {!Document.Obsolete} */
  Document.prototype.obsolete = Document.Obsolete.UNKNOWN;

  /** @type {number} */
  Document.prototype.lastStatusCheckTime_ = 0;

  /**
   * @this {!Document}
   * @param {string} key_combination.
   * @param {Object} command.
   */
  Document.prototype.bindKey = function (key_combination, command) {
    var key_code = Editor.parseKeyCombination(key_combination);
    var map = this.keymap;
    if (!map) {
      map = new Map();
      this.keymap = map;
    }
    map.set(key_code, command);
  };

  Document.prototype.close = function() {
    var document = this;
    if (!document.needSave()) {
      document.forceClose();
      return;
    }
    Editor.messageBox(null,
        Editor.localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
        MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
      .then(function(response_code) {
        switch (response_code) {
          case DialogItemId.NO:
            document.forceClose();
            break;
          case DialogItemId.YES:
            Editor.getFilenameForSave(null, document.filename)
              .then(function(filename) {
                document.save(filename).then(function() {
                  document.forceClose();
                });
              });
            break;
        }
      });
  };

  /**
   * @param {Unit} unit.
   * @param {number} position.
   * @return {number} new position.
   */
  Document.prototype.computeEndOf_ = function(unit, position) {
    var document = this;
    switch (unit) {
      case Unit.CHARACTER:
        throwUnsupportedUnit('endOf', 'CHARACTER');
      case Unit.DOCUMENT:
        return document.length;
      case Unit.LINE:
        while (position < document.length) {
          if (document.charCodeAt_(position) == Unicode.LF)
            return position;
          ++position;
        }
        return position;
      case Unit.PAGE:
        throwUnsupportedUnit('endOf', 'PAGE');
      case Unit.PARAGRAPH:
        throwUnsupportedUnit('endOf', 'PARAGRAPH');
      case Unit.SCREEN:
        throwUnsupportedUnit('endOf', 'SCREEN');
      case Unit.SENTENCE:
        throwNYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        throwUnsupportedUnit('endOf', 'WINDOW');
      case Unit.WORD: {
        if (position == document.length)
          return position;
        var word_class = wordClassAt(document, position);
        if (word_class == WordClass.BLANK)
          return position;
        for (;;) {
          ++position;
          if (position == document.length)
            return position;
          var word_class2 = wordClassAt(document, position);
          if (word_class != word_class2)
            return position;
        }
        return position;
      }
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  };

  /**
   * @this {!Document}
   * @param {Unit} unit.
   * @param {number} count.
   * @param {number} position.
   * @return {number}
   *
   * Word Motion:
   *                      forward             backward
   *  th|is is a word.    this |is a word.    |this is a word.
   *  this |is a word.    this is| a word.    |this is a word.
   */
  Document.prototype.computeMotion_ = function(unit, count, position) {
    var document = this;
    switch (unit) {
      case Unit.CHARACTER:
        if (count > 0)
          return Math.min(document.length, position + count);
        if (count < 0)
          return Math.max(0, position + count);
        return position;
      case Unit.LINE:
        if (count > 0) {
          for (var k = 0; k < count; ++k) {
            position = document.computeEndOf_(unit, position);
            if (position == document.length)
              return position;
            ++position;
          }
          return position;
        }
        if (count < 0) {
          for (var k = count; k < 0; ++k) {
            position = document.computeStartOf_(unit, position);
            if (!position)
              break;
          }
          return position;
        }
        return position;
      case Unit.WORD:
        if (count > 0) {
          if (position == document.length)
            return position;
          for (var k = 0; k < count; ++k) {
            var word_class = wordClassAt(document, position);
            for (;;) {
              ++position;
              if (position == document.length)
                return position;
              var word_class2 = wordClassAt(document, position);
              if (word_class == word_class2)
                continue;
              while (word_class2 == WordClass.BLANK) {
                ++position;
                if (position == document.length)
                  return position;
                word_class2 = wordClassAt(document, position);
              }
              return position;
            }
          }
          return position;
        }

        if (count < 0) {
          if (!position)
            return position;
          for (var k = count; k < 0; ++k) {
            --position;
            var word_class = wordClassAt(document, position);
            for (;;) {
              if (!position)
                return position;
              --position;
              var word_class2 = wordClassAt(document, position);
              if (word_class == word_class2)
                continue;
              if (word_class == WordClass.BLANK) {
                while (word_class2 == WordClass.BLANK) {
                  if (!position)
                    return position;
                  --position;
                  word_class2 = wordClassAt(document, position);
                }
              }
              ++position;
              break;
            }
          }
        }
        return position;
      default:
        throw TypeError('Invalid unit: ' + unit);
    }
  };

  /**
   * @param {Unit} unit.
   * @param {number} position.
   * @return {number} new position.
   */
  Document.prototype.computeStartOf_ = function(unit, position) {
    var document = this;
    switch (unit) {
      case Unit.CHARACTER:
        throwUnsupportedUnit('startOf', 'CHARACTER');
      case Unit.DOCUMENT:
        return 0;
      case Unit.LINE:
        while (position > 0) {
          --position;
          if (document.charCodeAt_(position) == Unicode.LF)
            return position + 1;
        }
        return position;
      case Unit.PAGE:
        throwUnsupportedUnit('startOf', 'PAGE');
      case Unit.PARAGRAPH:
        throwUnsupportedUnit('startOf', 'PARAGRAPH');
      case Unit.SCREEN:
        throwUnsupportedUnit('startOf', 'SCREEN');
      case Unit.SENTENCE:
        throwNYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        throwUnsupportedUnit('startof', 'WINDOW');
      case Unit.WORD: {
        if (!position)
          return position;
        var word_class = wordClassAt(document, position);
        if (word_class == WordClass.BLANK)
          return position;
        while (position) {
          --position;
          var word_class2 = wordClassAt(document, position);
          if (word_class != word_class2)
            return position + 1;
        }
        return position;
      }
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  };

  /**
   * @param {string} char_set
   * @param {number} count
   * @param {number} start
   * @return {number}
   */
  Document.prototype.computeWhile_ = function(char_set, count, start) {
    var char_code_set = new Set();
    for (var i = 0; i < char_set.length; ++i) {
      char_code_set.add(char_set.charCodeAt(i));
    }
    if (count > 0) {
      var end = Math.min(start + count, this.length);
      for (var position = start; position < end; ++position) {
        var char_code = this.charCodeAt_(position);
        if (!char_code_set.has(char_code))
          return position;
      }
      return end;
    }

    var end = Math.max(start + count, 0);
    for (var position = start; position > end; --position) {
      var char_code = this.charCodeAt_(position - 1);
      if (!char_code_set.has(char_code))
        return position;
    }
    return end;
  };

  Document.prototype.forceClose = function() {
    this.listWindows().forEach(function(window) {
      window.destroy();
    });
    Document.remove(this);
  };

  /**
   * @return {Array.<!DocumentWindow>}
   */
  Document.prototype.listWindows = function() {
    var document = this;
    var windows = [];
    EditorWindow.list.forEach(function(editorWindow) {
      editorWindow.children.forEach(function(window) {
        if ((window instanceof DocumentWindow) && window.document == document)
          windows.push(window);
      });
    });
    return windows;
  };

  /**
   * @param {string} filename A backing store file of document.
   * @return {!Document} A Document bound to filename
   */
  Document.open = function(filename) {
    var absolute_filename = FilePath.fullPath(filename);
    var present = findDocumentOnFile(absolute_filename);
    if (present)
      return present;
    var document = new Document(FilePath.basename(filename));
    document.filename = absolute_filename;
    return document;
  };

  /**
   * @param {string=} opt_filename
   * @return {!Promise.<number>}
   */
  Document.prototype.load = function(opt_filename) {
    var document = this;
    if (!arguments.length) {
      if (document.filename == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var filename = /** @type{string} */(opt_filename);
      var absolute_filename = FilePath.fullPath(filename);
      var present = findDocumentOnFile(absolute_filename);
      if (present !== this)
        throw filename + ' is already bound to ' + present;
      document.filename = absolute_filename;
    }

    var deferred = Promise.defer();
    document.load_(document.filename, function(error_code) {
      if (!error_code)
        deferred.resolve(error_code);
      else
        deferred.reject(error_code);
    });
    return deferred.promise.then(function(error_code) {
      console.log('load', document, 'error', error_code);
      document.parseFileProperties();
      var new_mode = Mode.chooseMode(document);
      if (new_mode.name != document.mode.name) {
        Editor.messageBox(null, 'Change mode to ' + new_mode.name,
                          MessageBox.ICONINFORMATION);
        document.mode = new_mode;
      }
      document.obsolete = Document.Obsolete.NO;
      document.lastStatusCheckTime_ = Date.now();
      document.listWindows().forEach(function(window) {
        if (window instanceof TextWindow) {
          console.log('Reset selection offset', document, window);
          window.selection.range.collapseTo(0);
        }
      });
      document.doColor_(Math.min(document.length, 1024 * 8));
    }).catch(function(exception) {
      console.log('Exception', exception, 'during loading into', document);
    });
  };

  /**
   * @return {boolean}
   */
  Document.prototype.needSave = function() {
    // TODO: We should use |document.notForSave|.
    return this.modified && !this.name.startsWith('*') &&
           FilePath.isValidFilename(this.filename);
  };

  /**
   * This function handles Emacs "File Variables" in the first line.
   * TODO(yosi) Support "Local Variables: ... End:".
   */
  Document.prototype.parseFileProperties = function() {
    var document = this;
    var first_line = new Range(document);
    first_line.endOf(Unit.LINE, Alter.EXTEND);
    var file_vars_matches = /-\*-\s+(.+?)\s+-\*-/.exec(first_line.text);
    if (!file_vars_matches)
      return;
    file_vars_matches[1].split(';').forEach(function(var_def) {
      var matches = /^\s*([^:\s]+)\s*:\s*(.+?)\s*$/.exec(var_def);
      if (!matches)
        return;
      document.properties.set(matches[1], matches[2]);
    });
  };

  /**
   * @param {string} name
   * @param {function()} callback
   * @param {!Object=} opt_receiver
   */
  Document.prototype.undoGroup = function(name, callback, opt_receiver) {
    var document = this;
    var receiver = arguments.length >= 3 ? opt_receiver : document;
    try {
      document.startUndoGroup_(name);
      callback.call(receiver);
    } finally {
      document.endUndoGroup_(name);
    }
  };
})();
