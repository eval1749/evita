// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  /** @const @type {Map.<string, string>} */
  var WORD_CLASS_MAP = (function() {
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

  function wordClassOf(char_code) {
    return WORD_CLASS_MAP.get(Unicode.UCD[char_code].category);
  }

  function wordClassAt(document, position) {
    return wordClassOf(document.charCodeAt_(position));
  }

  Document.prototype.close = function() {
    var document = this;
    if (!document.modified) {
      document.forceClose();
      return;
    }
    Editor.messageBox(null,
                      localizeText(IDS_ASK_SAVE, {name: document.name}),
                      localizeText(IDS_APP_TITLE),
                      MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
      .then(function(response_code) {
        switch (response_code) {
          case DialogIemId.NO:
            document.forceClose();
            break;
          case DialogIemId.YES:
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
        throwUnsupportedUnit('endOf', 'LINE');
      case Unit.PAGE:
        throwUnsupportedUnit('endOf', 'PAGE');
      case Unit.PARAGRAPH:
        while (position < document.length) {
          if (document.charCodeAt_(position) == '\n')
            return position;
          ++position;
        }
        return position;
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
        throwUnsupportedUnit('startOf', 'LINE');
      case Unit.PAGE:
        throwUnsupportedUnit('startOf', 'PAGE');
      case Unit.PARAGRAPH:
        while (position > 0) {
          --position;
          if (document.charCodeAt_(position) == '\n')
            return position + 1;
        }
        return position;
      case Unit.SCREEN:
        throwUnsupportedUnit('startOf', 'SCREEN');
      case Unit.SENTENCE:
        throwNYI('startof', 'SENTENCE');
      case Unit.WINDOW:
        throwUnsupportedUnit('startof', 'WINDOW');
      case Unit.WORD: {
        if (!position)
          return position;
        var word_class = wordClassOf(document.charCodeAt_(position));
        if (word_class == WordClass.BLANK)
          return position;
        while (position) {
          --position;
          var word_class2 = wordClassOf(document.charCodeAt_(position));
          if (word_class != word_class2)
            return position + 1;
        }
        return position;
      }
      default:
        throwInvalidUnit(unit);
    }
  };

  Document.prototype.forceClose = function() {
    this.listWindows().forEach(function(window) {
      window.destroy();
    });
    Document.remove(this);
  };

  /**
   * @return {Array.<TextWindow>}
   */
  Document.prototype.listWindows = function() {
    var document = this;
    var windows = [];
    EditorWindow.list.forEach(function(editorWindow) {
      editorWindow.children.forEach(function(window) {
        if ((window instanceof TextWindow) && window.document == document)
          windows.push(window);
      });
    });
    return windows;
  };

  /**
   * @return {boolean}
   */
  Document.prototype.needSave = function() {
    // TODO: We should use |document.notForSave|.
    return this.modified && FilePath.isValidFilename(this.filename);
  };

  /**
   * @param {string} A filename to load from.
   * @return {Document} A Document object contains contents of file.
   */
  Document.load = function(filename) {
    var absolute_filename = FilePath.fullPath(filename);
    var canonical_filename = absolute_filename.toLocaleLowerCase();
    var present = Document.list.find(function(document) {
      return document.filename.toLocaleLowerCase() == canonical_filename;
    });
    if (present)
      return present;
    var document = new Document(FilePath.basename(filename));
    document.load_(absolute_filename);
    return document;
  };
})();
