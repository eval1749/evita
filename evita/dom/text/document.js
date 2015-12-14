// Copyright (c) 2014 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file
'use strict';

(function() {
  /** @enum {string} */
  const WordClass = {BLANK: 'Z', PUNCTUATION: 'P', WORD: 'w'};

  /** @const @type {Map.<string, WordClass>} */
  const WORD_CLASS_MAP = (function() {
    /** @param {string} name */
    function mapCategoryToWordClass(name) {
      let wordClass = name.charAt(0);
      if (wordClass === 'L' || wordClass === 'N')
        return WordClass.WORD;
      if (wordClass === 'S')
        return WordClass.PUNCTUATION;
      return wordClass;
    }
    const map = new Map();
    Unicode.CATEGORY_SHORT_NAMES.forEach(function(name) {
      map.set(name, mapCategoryToWordClass(name));
    });
    return map;
  })();

  /** @type {Keymap} */
  Document.prototype.keymap_;

  /** @type {Mode} */
  Document.prototype.mode_;

  /** @type {string} */
  Document.prototype.name_;

  function throwInvalidUnit(unit) { throw 'Invalid unit: ' + unit; }

  function throwNYI(name, unit) {
    throw 'NYI Range.prototype.' + name + ' Unit.' + unit;
  }

  function throwUnsupportedUnit(name, unit) {
    throw 'Range.prototype.' + name + ' does not support Unit.' + unit;
  }

  /**
   * @param {number} charCode
   * @return {string|null}
   *
   * Note: We should not treat TAB and LF as blank. If we do so,
   * |Ctrl+Backspace| at start of line, removes newline at end of the previous
   * line
   */
  function wordClassOf(charCode) {
    return WORD_CLASS_MAP.get(Unicode.UCD[charCode].category) || null;
  }

  /**
   * @param {!Document} document
   * @param {number} position
   * @return {string|null}
   */
  function wordClassAt(document, position) {
    return wordClassOf(document.charCodeAt_(position));
  }

  /**
   * @param {!Document} document
   * @param {number} position
   * @return {string|null}
   */
  function wordClassBefore(document, position) {
    return wordClassOf(document.charCodeAt_(position - 1));
  }

  /**
   * @this {!Document}
   * @param {string} keyCombination
   * @param {!Function|!Map} command
   */
  function bindKey(keyCombination, command) {
    let keyCode = Editor.parseKeyCombination(keyCombination);
    this.keymap.set(keyCode, command);
  }

  /**
   * @param {!Document} document
   * @param {number} offset
   * @return {number} An offset of end of line containing |offset|.
   */
  function computeEndOfLine(document, offset) {
    while (offset < document.length) {
      if (document.charCodeAt_(offset) === Unicode.LF)
        return offset;
      ++offset;
    }
    return offset;
  }

  /**
   * @param {!Document} document
   * @param {number} offset
   * @return {number} An offset of end of word containing |offset|.
   */
  function computeEndOfWord(document, offset) {
    if (offset === document.length)
      return offset;
    let word_class = wordClassAt(document, offset);
    if (word_class === WordClass.BLANK) {
      if (offset !== 0 &&
          wordClassBefore(document, offset) !== WordClass.BLANK) {
        // We're already at end of word
        return offset;
      }
      // Skip spaces
      while (offset < document.length) {
        word_class = wordClassAt(document, offset);
        if (word_class !== WordClass.BLANK)
          break;
        ++offset;
      }
    }
    while (offset < document.length) {
      if (wordClassAt(document, offset) !== word_class)
        break;
      ++offset;
    }
    return offset;
  }

  /**
   * @this {!Document}
   * @param {Unit} unit
   * @param {number} offset
   * @return {number} An offset of end of |unit|.
   */
  function computeEndOf(unit, offset) {
    let document = this;
    switch (unit) {
      case Unit.DOCUMENT:
        return document.length;
      case Unit.LINE:
        return computeEndOfLine(this, offset);
      case Unit.WORD:
        return computeEndOfWord(this, offset);
      case Unit.CHARACTER:
      case Unit.PARAGRAPH:
      case Unit.PAGE:
      case Unit.SCREEN:
      case Unit.SENTENCE:
      case Unit.WINDOW:
        throwUnsupportedUnit('computeEndOf', unit);
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  }

  /**
   * @param {!Document} document
   * @param {number} count
   * @param {number} offset
   * @return {number}
   */
  function computeMotionBracket(document, count, offset) {
    const position = new TextPosition(document, offset);
    position.move(Unit.BRACKET, count);
    return position.offset;
  }

  /**
   * @param {!Document} document
   * @param {number} count
   * @param {number} offset
   * @return {number}
   */
  function computeMotionCharacter(document, count, offset) {
    if (count > 0)
      return Math.min(document.length, offset + count);
    if (count < 0)
      return Math.max(0, offset + count);
    return offset;
  }

  /**
   * @param {!Document} document
   * @param {number} count
   * @param {number} offset
   * @return {number}
   */
  function computeMotionLine(document, count, offset) {
    if (count > 0) {
      for (let k = 0; k < count; ++k) {
        offset = document.computeEndOf_(Unit.LINE, offset);
        if (offset === document.length)
          return offset;
        ++offset;
      }
      return offset;
    }
    if (count < 0) {
      for (let k = count; k < 0; ++k) {
        offset = document.computeStartOf_(Unit.LINE, offset);
        if (offset === 0)
          break;
      }
      return offset;
    }
    return offset;
  }

  /**
   * @param {!Document} document
   * @param {number} count
   * @param {number} offset
   * @return {number}
   *
   *                      forward             backward
   *  th|is is a word.    this |is a word.    |this is a word
   *  this |is a word.    this is| a word.    |this is a word
   */
  function computeMotionWord(document, count, offset) {
    if (count > 0) {
      if (offset === document.length)
        return offset;
      for (let k = 0; k < count; ++k) {
        let wordClass = wordClassAt(document, offset);
        for (;;) {
          ++offset;
          if (offset === document.length)
            break;
          let wordClass2 = wordClassAt(document, offset);
          if (wordClass === wordClass2)
            continue;
          while (wordClass2 === WordClass.BLANK) {
            ++offset;
            if (offset === document.length)
              return offset;
            wordClass2 = wordClassAt(document, offset);
          }
          break;
        }
      }
      return offset;
    }

    if (count < 0) {
      if (offset === 0)
        return offset;
      for (let k = count; k < 0; ++k) {
        --offset;
        let wordClass = wordClassAt(document, offset);
        for (;;) {
          if (offset === 0)
            return offset;
          --offset;
          let wordClass2 = wordClassAt(document, offset);
          if (wordClass === wordClass2)
            continue;
          if (wordClass === WordClass.BLANK) {
            while (wordClass2 === WordClass.BLANK) {
              if (offset === 0)
                return offset;
              --offset;
              wordClass2 = wordClassAt(document, offset);
            }
          }
          ++offset;
          break;
        }
      }
    }
    return offset;
  }

  /**
   * @this {!Document}
   * @param {number} count
   * @param {number} offset
   * @return {number}
   */
  function computeMotion(unit, count, offset) {
    switch (unit) {
      case Unit.BRACKET:
        return computeMotionBracket(this, count, offset);
      case Unit.CHARACTER:
        return computeMotionCharacter(this, count, offset);
      case Unit.LINE:
        return computeMotionLine(this, count, offset);
      case Unit.WORD:
        return computeMotionWord(this, count, offset);
      default:
        throw TypeError('Invalid unit: ' + unit);
    }
  }

  /**
   * @param {!Document} document
   * @param {number} offset
   * @return {number} An offset of start of line containing |offset|.
   */
  function computeStartOfLine(document, offset) {
    while (offset > 0) {
      --offset;
      if (document.charCodeAt_(offset) === Unicode.LF)
        return offset + 1;
    }
    return offset;
  }

  /**
   * @param {!Document} document
   * @param {number} offset
   * @return {number} An offset of start of word containing |offset|.
   */
  function computeStartOfWord(document, offset) {
    if (offset === 0)
      return offset;
    let wordClass = offset === document.length ?
                         WordClass.BLANK :
                         wordClassAt(document, offset);
    // Find character class of word
    while (wordClass === WordClass.BLANK) {
      --offset;
      if (offset === 0)
        return offset;
      wordClass = wordClassAt(document, offset);
    }
    // Skip word
    while (offset && wordClassAt(document, offset - 1) === wordClass)
      --offset;
    return offset;
  }

  /**
   * @this {!Document}
   * @param {Unit} unit
   * @param {number} offset
   * @return {number} An offset of start of |unit|.
   */
  function computeStartOf(unit, offset) {
    switch (unit) {
      case Unit.DOCUMENT:
        return 0;
      case Unit.LINE:
        return computeStartOfLine(this, offset);
      case Unit.WORD:
        return computeStartOfWord(this, offset);
      case Unit.CHARACTER:
      case Unit.PAGE:
      case Unit.PARAGRAPH:
      case Unit.SCREEN:
      case Unit.SENTENCE:
      case Unit.WINDOW:
        throwUnsupportedUnit('computeStartOf', unit);
      default:
        throwInvalidUnit(unit);
        return 0;
    }
  }

  /**
   * @this {!Document}
   * @param {string} charSet
   * @param {number} count
   * @param {number} start
   * @return {number}
   */
  function computeWhile(charSet, count, start) {
    const charCodeSet = new Set();
    for (let i = 0; i < charSet.length; ++i)
      charCodeSet.add(charSet.charCodeAt(i));

    if (count > 0) {
      const maxOffset = Math.min(start + count, this.length);
      for (let offset = start; offset < maxOffset; ++offset) {
        const charCode = this.charCodeAt_(offset);
        if (!charCodeSet.has(charCode))
          return offset;
      }
      return maxOffset;
    }

    const minOffset = Math.max(start + count, 0);
    for (let offset = start; offset > minOffset; --offset) {
      const charCode = this.charCodeAt_(offset - 1);
      if (!charCodeSet.has(charCode))
        return offset;
    }
    return minOffset;
  }

  /**
   * @this {!Document}
   * @param {number} hint
   */
  function doColor_(hint) {
    if (!this.mode)
      return;
    this.mode.doColor(this, hint);
  }

  /**
   * @this {!Document}
   * @return {boolean}
   */
  function getDocumentModified() {
    return this.revision_ !== this.savedRevision_;
  }

  /**
   * @this {!Document}
   *
   * Returns generator object which returns line, including newline
   * character, in a document.
   */
  function* lines() {
    const range = new Range(this);
    while (range.start < this.length) {
      range.moveEnd(Unit.LINE);
      yield range.text;
      range.collapseTo(range.end);
    }
  }

  /**
   * @this {!Document}
   * @return {Array.<!DocumentWindow>}
   */
  function listWindows() {
    const windows = [];
    for (let editorWindow of EditorWindow.list) {
      for (let window of editorWindow.children) {
        if ((window instanceof DocumentWindow) && window.document === this)
          windows.push(window);
      }
    }
    return windows;
  };

  /**
   * @this {!Document}
   * @param {boolean} newModified
   */
  function setDocumentModified(newModified) {
    if (this.modified === !!newModified)
      return;
    this.savedRevision_ = newModified ? -1 : this.revision_;
  }

  /**
   * @this {!Document}
   * @param {string} name
   * @param {function()} callback
   * @param {!Object=} opt_receiver
   */
  function undoGroup(name, callback, opt_receiver) {
    const document = this;
    const receiver = opt_receiver || document;
    try {
      document.startUndoGroup_(name);
      callback.call(receiver);
    } finally {
      document.endUndoGroup_(name);
    }
  }

  /** @type {!Map.<string, !Document>} */
  const documentNameMap = new Map();

  /** @type {!Set.<!DocumentObserverCallback>} */
  let documentObservers = new Set();

  /** @type {!Set.<!DocumentObserverCallback>} */
  let internalDocumentObservers = new Set();

  /** @param {!DocumentObserverCallback} callback */
  function addObserver(callback) {
    documentObservers.add(callback);
  }

  /**
   * @param {string} name
   * @return {Document}
   */
  function findDocument(name) {
    return documentNameMap.get(name) || null;
  }

  /**
   * @return {!Array.<!Document>}
   */
  function listDocument() {
    // TODO(eval1749): Once closure compiler support spreading, we should write
    // |return [... documentNameMap.values()];|
    const result = [];
    for (let document of documentNameMap.values())
      result.push(document);
    return result;
  }

  /**
   * @param {string} candidate
   * @return {string}
   */
  function makeUniqueName(candidate) {
    let uniqueName = candidate;
    const lastDot = candidate.lastIndexOf('.');
    const head = lastDot > 0 ? candidate.substr(0, lastDot) : candidate;
    const tail = lastDot > 0 ? candidate.substr(lastDot) : '';
    let body = '';
    let count = 1;
    for (;;) {
      const uniqueName = head + body + tail;
      if (!findDocument(uniqueName))
        return uniqueName;
      ++count;
      body = ` (${count})`
    }
  }

  /**
   * @param {string} name
   * @return {!Document}
   */
  function newDocument(name) {
    const document = new Document();
    document.name_ = makeUniqueName(name);
    documentNameMap.set(document.name, document);
    for (let observer of documentObservers)
      observer.call(Document, 'add', document);
    return document;
  }

  /** @param {!Document} document */
  function removeDocument(document) {
    if (!documentNameMap.has(document.name))
      throw new Error(`${document.name} isn't in list`);
    documentNameMap.delete(document.name);
    for (let observer of documentObservers)
      observer.call(Document, 'remove', document);
  }

  /** @param {!DocumentObserverCallback} callback */
  function removeObserver(callback) {
    documentObservers.delete(callback);
  }

  /**
   * @this {!Document}
   * @param {string} newName
   */
  function renameTo(newName) {
    documentNameMap.delete(this.name);
    this.name_ = makeUniqueName(newName);
    documentNameMap.set(this.name_, this);
  }

  $initialize(function() {
    documentNameMap.clear();
    if (internalDocumentObservers.size === 0) {
      internalDocumentObservers = new Set(documentObservers);
      return;
    }
    documentObservers = new Set(internalDocumentObservers);
  });

  Object.defineProperties(Document, {
    addObserver: {value: addObserver},
    find: {value: findDocument},
    list: {get: () => listDocument()},
    new: {value: newDocument},
    remove: {value: removeDocument},
    removeObserver: {value: removeObserver},
  });

  Object.defineProperties(Document.prototype, {
    fileName: {value: '', writable: true},
    keymap: {
      get: /** @return {!Keymap} */ function() {
        if (!this.keymap_)
          this.keymap_ = new Map();
        return this.keymap_;
      }
    },
    keymap_: {value: null, writable: true},
    lastWriteTime: {value: new Date(0), writable: true},
    mode: {
      get: /** @return {?Mode} */ function() { return this.mode_; },
      set: /** @param {?Mode} mode */ function(mode) {
        if (this.mode_)
          this.mode_.detach();
        this.mode_ = mode;
        if (mode)
          mode.attach(this);
      }
    },
    mode_: {value: null, writable: true},
    modified: {get: getDocumentModified, set: setDocumentModified},
    name_: {value: '', writable: true},
    name: {get: function() { return this.name_; }},
    newline: {value: 0, writable: true},
    properties: {
      get: /** @return {!Map} */ function() {
        if (!this.properties_)
          this.properties_ = new Map();
        return this.properties_;
      }
    },
    properties_: {value: null, writable: true},
    state: {value: 0, writable: true},
    savedRevision_: {value: 0, writable: true},

    bindKey: {value: bindKey},
    computeEndOf_: {value: computeEndOf},
    computeMotion_: {value: computeMotion},
    computeStartOf_: {value: computeStartOf},
    computeWhile_: {value: computeWhile},
    doColor_: {value: doColor_},
    lines: {get: lines},
    listWindows: {value: listWindows},
    renameTo: {value: renameTo},
    undoGroup: {value: undoGroup}
  });
})();
