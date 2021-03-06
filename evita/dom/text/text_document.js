// Copyright (c) 2014 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

goog.require('unicode');

goog.scope(function() {
/** @enum {string} */
const WordClass = {
  BLANK: 'Z',
  PUNCTUATION: 'P',
  WORD: 'w'
};

/** @const @type {Map<string, WordClass>} */
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

/** @type {string} */
TextDocument.prototype.name_;

function throwInvalidUnit(unit) {
  throw 'Invalid unit: ' + unit;
}

function throwNYI(name, unit) {
  throw 'NYI TextRange.prototype.' + name + ' Unit.' + unit;
}

function throwUnsupportedUnit(name, unit) {
  throw 'TextRange.prototype.' + name + ' does not support Unit.' + unit;
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
  return WORD_CLASS_MAP.get(unicode.categoryOf(charCode)) || null;
}

/**
 * @param {!TextDocument} document
 * @param {number} position
 * @return {string|null}
 */
function wordClassAt(document, position) {
  return wordClassOf(document.charCodeAt(position));
}

/**
 * @param {!TextDocument} document
 * @param {number} position
 * @return {string|null}
 */
function wordClassBefore(document, position) {
  return wordClassOf(document.charCodeAt(position - 1));
}

/**
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {number} An offset of end of line containing |offset|.
 */
function computeEndOfLine(document, offset) {
  while (offset < document.length) {
    if (document.charCodeAt(offset) === Unicode.LF)
      return offset;
    ++offset;
  }
  return offset;
}

/**
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {number} An offset of end of word containing |offset|.
 */
function computeEndOfWord(document, offset) {
  if (offset === document.length)
    return offset;
  let word_class = wordClassAt(document, offset);
  if (word_class === WordClass.BLANK) {
    if (offset !== 0 && wordClassBefore(document, offset) !== WordClass.BLANK) {
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
 * @this {!TextDocument}
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
 * @param {!TextDocument} document
 * @param {number} count
 * @param {number} offset
 * @return {number}
 */
function computeMotionBracket(document, count, offset) {
  if (count === 0)
    return count;
  if (count > 0)
    return text.findBracketForward(document, offset);
  return text.findBracketBackward(document, offset);
}

/**
 * @param {!TextDocument} document
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
 * @param {!TextDocument} document
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
 * @param {!TextDocument} document
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
 * @this {!TextDocument}
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
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {number} An offset of start of line containing |offset|.
 */
function computeStartOfLine(document, offset) {
  while (offset > 0) {
    --offset;
    if (document.charCodeAt(offset) === Unicode.LF)
      return offset + 1;
  }
  return offset;
}

/**
 * @param {!TextDocument} document
 * @param {number} offset
 * @return {number} An offset of start of word containing |offset|.
 */
function computeStartOfWord(document, offset) {
  if (offset === 0)
    return offset;
  let wordClass = offset === document.length ? WordClass.BLANK :
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
 * @this {!TextDocument}
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
 * @this {!TextDocument}
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
      const charCode = this.charCodeAt(offset);
      if (!charCodeSet.has(charCode))
        return offset;
    }
    return maxOffset;
  }

  const minOffset = Math.max(start + count, 0);
  for (let offset = start; offset > minOffset; --offset) {
    const charCode = this.charCodeAt(offset - 1);
    if (!charCodeSet.has(charCode))
      return offset;
  }
  return minOffset;
}

/**
 * @this {!TextDocument}
 * @return {boolean}
 */
function getTextDocumentModified() {
  return this.revision_ !== this.savedRevision_;
}

/**
 * @this {!TextDocument}
 * @return {!Generator<string>}
 *
 * Returns generator object which returns line, including newline
 * character, in a document.
 */
function * lines() {
  const range = new TextRange(this);
  while (range.start < this.length) {
    range.moveEnd(Unit.LINE);
    yield range.text;
    range.collapseTo(range.end);
  }
}

/**
 * @this {!TextDocument}
 * @return {Array<!TextWindow>}
 */
function listWindows() {
  const windows = [];
  for (let editorWindow of EditorWindow.list) {
    for (let window of editorWindow.children) {
      if ((window instanceof TextWindow) && window.document === this)
        windows.push(window);
    }
  }
  return windows;
};

/**
 * @this {!TextDocument}
 * @param {boolean} newModified
 */
function setTextDocumentModified(newModified) {
  if (this.modified === !!newModified)
    return;
  this.savedRevision_ = newModified ? -1 : this.revision_;
}

/**
 * @this {!TextDocument}
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

/** @type {!Map<string, !TextDocument>} */
const documentNameMap = new Map();

/** @type {!Set<!TextDocumentSetObserver>} */
let documentSetObservers = new Set();

/** @type {!Set<!TextDocumentSetObserver>} */
let internalTextDocumentSetObservers = new Set();

/** @param {!TextDocumentSetObserver} callback */
function addObserver(callback) {
  documentSetObservers.add(callback);
}

/**
 * @param {string} name
 * @return {TextDocument}
 */
function findTextDocument(name) {
  return documentNameMap.get(name) || null;
}

/**
 * @return {!Array<!TextDocument>}
 */
function listTextDocument() {
  return [...documentNameMap.values()];
}

/**
 * @param {string} candidate
 * @return {string}
 */
function makeUniqueName(candidate) {
  const lastDot = candidate.lastIndexOf('.');
  const head = lastDot > 0 ? candidate.substr(0, lastDot) : candidate;
  const tail = lastDot > 0 ? candidate.substr(lastDot) : '';
  let body = '';
  let count = 1;
  for (;;) {
    const uniqueName = head + body + tail;
    if (!findTextDocument(uniqueName))
      return uniqueName;
    ++count;
    body = ` (${count})`;
  }
}

/**
 * @param {!TextDocument} document
 */
function addTextDocument(document) {
  if (documentNameMap.has(document.name))
    throw new Error(`Document ${document.name} already exists`);
  documentNameMap.set(document.name, document);
  for (const observer of documentSetObservers)
    observer.didAddTextDocument(document);
}

/**
 * @param {string} name
 * @return {!TextDocument}
 */
function newTextDocument(name) {
  const document = new TextDocument();
  document.name_ = makeUniqueName(name);
  addTextDocument(document);
  return document;
}

/** @param {!TextDocument} document */
function removeTextDocument(document) {
  if (!documentNameMap.has(document.name))
    throw new Error(`${document.name} isn't in list`);
  documentNameMap.delete(document.name);
  document.dispatchEvent(new Event('remove'));
  for (const observer of documentSetObservers)
    observer.didRemoveTextDocument(document);
}

/** @param {!TextDocumentSetObserver} callback */
function removeObserver(callback) {
  documentSetObservers.delete(callback);
}

/**
 * @this {!TextDocument}
 * @param {string} newName
 */
function renameTo(newName) {
  documentNameMap.delete(this.name);
  this.name_ = makeUniqueName(newName);
  documentNameMap.set(this.name_, this);
}

core.Initializer.register(function() {
  documentNameMap.clear();
  if (internalTextDocumentSetObservers.size === 0) {
    internalTextDocumentSetObservers = new Set(documentSetObservers);
    return;
  }
  documentSetObservers = new Set(internalTextDocumentSetObservers);
});

Object.defineProperties(TextDocument, {
  add: {value: addTextDocument},
  addObserver: {value: addObserver},
  find: {value: findTextDocument},
  list: {get: listTextDocument},
  new: {value: newTextDocument},
  remove: {value: removeTextDocument},
  removeObserver: {value: removeObserver},
});

/**
 * @this {!TextDocument}
 * @return {string}
 */
function getName() {
  return this.name_;
}

/**
 * @this {!TextDocument}
 * @return {!Map}
 */
function getProperties() {
  if (!this.properties_)
    this.properties_ = new Map();
  return this.properties_;
}

/**
 * @this {!TextDocument}
 * @return {string}
 */
function toString() {
  return `TextDocument('${this.name}')`;
}

Object.defineProperties(TextDocument.prototype, {
  fileName: {value: '', writable: true},
  lastWriteTime: {value: new Date(0), writable: true},
  modified: {get: getTextDocumentModified, set: setTextDocumentModified},
  name_: {value: '', writable: true},
  name: {get: getName},
  newline: {value: 0, writable: true},
  properties: {get: getProperties},
  properties_: {value: null, writable: true},
  state: {value: 0, writable: true},
  savedRevision_: {value: 0, writable: true},

  computeEndOf_: {value: computeEndOf},
  computeMotion_: {value: computeMotion},
  computeStartOf_: {value: computeStartOf},
  computeWhile_: {value: computeWhile},
  lines: {get: lines},
  listWindows: {value: listWindows},
  renameTo: {value: renameTo},
  toString: {value: toString},
  undoGroup: {value: undoGroup}
});

/** @type {!Generator<string>} */
TextDocument.prototype.lines;

/**
 * @param {string} name
 * @param {function()} callback
 * @param {!Object=} opt_receiver
 */
TextDocument.prototype.undoGroup;

});
