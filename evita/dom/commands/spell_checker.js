// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @enum {number}
 */
global.Spelling = {
  NONE: 0,
  CORRECT: 1,
  MISSPELLED: 2,
  DEBUG: 3 // green wave
};

/**
 * Spell checker for specified document. Each document is associated to
 * separate |SpellChecker| instance.
 *
 * @constructor
 * @param {!Document} document
 */
function SpellChecker(document) {
  this.coldEnd = document.length;
  this.coldOffset = 0;
  this.freeRanges_ = new Array(SpellChecker.MAX_CHECKING);
  for (var i = 0; i < this.freeRanges_.length; ++i) {
    this.freeRanges_[i] = new Range(document);
  }
  this.hotOffset = document.length;
  this.mutationObserver_ = new MutationObserver(
      this.mutationCallback.bind(this));
  this.range_ = new Range(document);
  this.timer_ = new RepeatingTimer();

  var spellChecker = this;
  document.addEventListener(Event.Names.ATTACH, function(event) {
    event.view.addEventListener(Event.Names.BLUR,
        spellChecker.didBlurWindow.bind(spellChecker));
    event.view.addEventListener(Event.Names.FOCUS,
        spellChecker.didFocusWindow.bind(spellChecker));
  });

  function setupMutationObserver() {
    spellChecker.mutationObserver_.observe(document, {summary: true});
  }

  // Ignore document mutation during loading contents.
  document.addEventListener(Event.Names.BEFORELOAD, function() {
    spellChecker.mutationObserver_.disconnect();
    spellChecker.timer_.stop();
  });

  // Restart spell checking from start of document after loading.
  document.addEventListener(Event.Names.LOAD, function() {
    setupMutationObserver();
    spellChecker.coldOffset = 0;
    spellChecker.coldEnd = document.length;
    spellChecker.hotOffset = 0;
  });

  setupMutationObserver();
}

/** @const @type {number} */
SpellChecker.CHECK_INTERVAL_LIMIT = 1000;

/** @const @type {number} */
SpellChecker.CHECK_INTERVAL_MS = 200;

/** @const @type {number} */
SpellChecker.MAX_CHECKING = 10;

/** @const @type {number} */
SpellChecker.MAX_WORD_LENGTH = 20;

/** @const @type {number} */
SpellChecker.MIN_WORD_LENGTH = 3;

/** @const @type {string} */
SpellChecker.PROPERTY = 'SpellChecker';

/** @const @type {!RegExp} */
SpellChecker.RE_WORD = new RegExp('^[A-Za-z][a-z]{' +
    (SpellChecker.MIN_WORD_LENGTH - 1) + ',' +
    (SpellChecker.MAX_WORD_LENGTH - 1) + '}$');

/**
 * @typedef{{
 *  lastUsedTime: !Date,
 *  promise: ?Promise,
 *  spelling: !Spelling,
 *  useCount: number,
 *  word: string
 * }}
 */
SpellChecker.SpellingResult

/** @type {!Set.<string>} */
SpellChecker.keywords;
Object.defineProperty(SpellChecker, 'keywords', {value: new Set()});

/** @type {number} */
SpellChecker.numberOfChecking;

/** @type {number} */
SpellChecker.prototype.coldEnd;

/** @type {number} */
SpellChecker.prototype.coldOffset;

/**
 * @type {!Array.<!Range>}
 * List of ranges can be used for checking spelling.
 */
SpellChecker.prototype.freeRanges_;

/** @type {number} */
SpellChecker.prototype.hotOffset;

/** @type {!MutationObserver} */
SpellChecker.prototype.mutationObserver_;

/**
 * @type {!Range}
 * A Range object for scanning document.
 */
SpellChecker.prototype.range_;

/** @type {!RepeatingTimer} */
SpellChecker.prototype.timer_;

/**
 * @param {!Range} wordRange
 * Checks then mark word as correct or misspelled.
 */
SpellChecker.prototype.checkSpelling = function(wordRange) {
  var spellChecker = this;
  if (!spellChecker.freeRanges_.length ||
      SpellChecker.numberOfChecking >= SpellChecker.MAX_CHECKING) {
    return false;
  }
  ++SpellChecker.numberOfChecking;

  var markerRange = spellChecker.freeRanges_.pop();
  markerRange.collapseTo(wordRange.start);
  markerRange.end = wordRange.end;
  var wordToCheck = markerRange.text;
  SpellChecker.checkSpelling(wordToCheck).then(function(state) {
    --SpellChecker.numberOfChecking;
    if (state.word == markerRange.text)
      spellChecker.markWord(markerRange, state.spelling);
    spellChecker.freeRanges_.push(markerRange);
  }).catch(function(e) { console.log(e.stack); });
  return true;
};

// Cleanup resources used by spell checker.
SpellChecker.prototype.destroy = function() {
  this.timer_.stop();
};

// Spell checker is stopped when window loses focus.

SpellChecker.prototype.didBlurWindow = function(event) {
  this.timer_.stop();
};

// Check spelling in hot region and cold region. When we check all words in
// cold region, |coldOffset| >= |coldEnd|, we check all the words in the
// document.
//
// Cold and hot regions are updated in mutation observer callback by using
// minimum changed offset.
//
//  * Cold region: |coldOffset| to |coldEnd|.
//  * Hot region: |hotOffset| to end of document.
//
//
//         |...........|....................|
//  offset 0           minimum change       document.length
//         === cold === ======= hot ========
//
// Word scanner, |scan()|, stops hot word which contains caret not to check
// spell incomplete word.
//
// Note: To reduce misspelling in source code, e.g. function name, and variable
// names, we don't check character syntax.
//
SpellChecker.prototype.didFireTimer = function() {
  var range = this.range_;
  var document = range.document;
  var maxOffset = document.length;

  // |didFireTimer| can be called before handling mutation callback, we should
  // make sure offsets don't exceed |document.length|.
  this.coldOffset = Math.min(this.coldOffset, maxOffset);
  this.coldEnd = Math.min(this.coldEnd, maxOffset);

  /**
   * @return {number}
   * Returns caret offset as hottest offset if window has focus.
   */
  function getHottestOffset() {
    if (!document.modified) {
      // If document isn't modified, document is just loaded or undo initial
      // change.
      return -1;
    }
    if (!(Window.focus instanceof TextWindow))
      return -1;
    if (Window.focus.document != document)
      return -1;
    var range = Window.focus.selection.range;
    return range.collapsed ? range.start : -1;
  }

  var hottestOffset = getHottestOffset();

  /**
   * @param {!Range} range
   * @return {boolean}
   */
  function isCheckableWord(range) {
    var length = range.length;
    if (length < SpellChecker.MIN_WORD_LENGTH ||
        length > SpellChecker.MAX_WORD_LENGTH) {
      return false;
    }
    if (hottestOffset >= range.start && hottestOffset <= range.end) {
      // The word is still changing, we ignore it.
      return false;
    }
    var category = Unicode.UCD[document.charCodeAt_(range.start)].category;
    // TODO(eval1749): When we support spell checking other than English, we
    // should make other Unicode general categories are valid.
    if (category != 'Lu' && category != 'Ll')
      return false;
    if (document.syntaxAt(range.start) == 'keyword')
      return false;
    var word = range.text;
    if (SpellChecker.keywords.has(word))
      return false;
    return SpellChecker.RE_WORD.test(word)
  }

  var numberOfChecked = 0;

  /**
   * @param {!SpellChecker} spellChecker
   * @param {number} start
   * @param {number} end
   * @return {number} Text offset for next scanning.
   */
  function scan(spellChecker, start, end) {
    if (SpellChecker.numberOfChecking >= SpellChecker.MAX_CHECKING)
      return start;
    range.collapseTo(start);
    range.startOf(Unit.WORD);
    while (numberOfChecked < SpellChecker.CHECK_INTERVAL_LIMIT &&
           range.start < end && spellChecker.freeRanges_.length) {
      var restartOffset = range.start;
      range.endOf(Unit.WORD, Alter.EXTEND);
      range.setSpelling(Spelling.NONE);
      if (range.end == maxOffset) {
        // Word seems not to be completed yet. Spell checker will sleep
        // until document is changed.
        numberOfChecked += range.length;
        return restartOffset;
      }

      // To reduce garbage collection, we check word in |wordRange| is whether
      // suitable for checking spelling or not.
      if (isCheckableWord(range) && !spellChecker.checkSpelling(range))
        return restartOffset;

      numberOfChecked += range.length;

      // Move to next word.
      // <#>include     => #|include
      // foo<;>\n   bar => foo;\n  |bar
      var wordStart = range.start;
      range.collapseTo(wordStart);
      range.move(Unit.WORD);
    }
    return range.start;
  }

  if (this.hotOffset < maxOffset)
    this.hotOffset = scan(this, this.hotOffset, maxOffset);
  this.coldOffset = scan(this, this.coldOffset, this.coldEnd);

  var rest = maxOffset - this.hotOffset + this.coldEnd - this.coldOffset;
  if (rest > 0)
    return;
  this.timer_.stop();
};

// Spell checking is started when window is focused.
SpellChecker.prototype.didFocusWindow = function() {
  this.startTimeIfNeeded();
};

/**
 * @param {!Range} wordRange
 * @param {Spelling} mark
 *
 * Mark first |word| without misspelled marker in document with |mark|.
 */
SpellChecker.prototype.markWord = function(wordRange, mark) {
  if (mark == Spelling.CORRECT) {
    return;
  }
  // TODO(eval1749): We should not mark for word in source code.
  wordRange.setSpelling(mark);
};

/**
 * @this {!SpellChecker}
 * @param {!Array.<!MutationRecord>} mutations
 * @param {!MutationObserver} observer
 *
 * Resets hot offset to minimal changed offset and kicks word scanner.
 */
SpellChecker.prototype.mutationCallback = function(mutations, observer) {
  var range = this.range_;
  var document = range.document;
  /** @type {number} */
  var minOffset = mutations.reduce(function(previousValue, mutation) {
    return Math.min(previousValue, mutation.offset);
  }, document.length);
  this.coldOffset = Math.min(this.coldOffset, minOffset);
  this.coldEnd = minOffset;
  this.hotOffset = minOffset;

  var hasFocus = document.listWindows().some(function(window) {
    return Window.focus == window;
  });
  if (hasFocus)
    this.startTimeIfNeeded();
};

SpellChecker.prototype.startTimeIfNeeded = function() {
  if (this.timer_.isRunning)
    return;
  this.timer_.start(SpellChecker.CHECK_INTERVAL_MS, this.didFireTimer, this);
};

/**
 * @type {!Map.<string, SpellChecker.SpellingResult>}
 *
 * Note: We expose |SpellChecker.wordStateMap| for debugging and testing
 * purpose.
 */
SpellChecker.wordStateMap = new Map();

/**
 * @param {string} wordToCheck
 * @return {!Promise.<SpellChecker.SpellingResult>}
 */
SpellChecker.checkSpelling = function(wordToCheck) {
  var present = SpellChecker.wordStateMap.get(wordToCheck);
  if (present) {
    present.lastUsedTime = new Date();
    ++present.useCount;
    return present.promise ? present.promise : Promise.resolve(present);
  }

  var state = {
    lastUsedTime: new Date(),
    promise: null,
    useCount: 1,
    word: wordToCheck,
  };
  SpellChecker.wordStateMap.set(wordToCheck, state);
  state.promise = Editor.checkSpelling(wordToCheck).then(
    function(isCorrect) {
      state.lastUsedTime = new Date(),
      state.promise = null;
      state.spelling = isCorrect ? Spelling.CORRECT : Spelling.MISSPELLED;
      return state;
    });
  return state.promise;
};

// When document is created/destructed, we install/uninstall spell checker
// to/from document.
Document.addObserver(function(action, document) {
  /** @param {!Document} document */
  function installSpellChecker(document) {
    // TODO(eval1749): We should have generic way to disable spell checking for
    // document.
    if (document.name == '*javascript*')
      return;
    var spellChecker = new SpellChecker(document);
    document.properties.set(SpellChecker.PROPERTY, spellChecker);
  }

  /** @param {!Document} document */
  function uninstallSpellChecker(document) {
    var spellChecker = document.properties.get(SpellChecker.PROPERTY);
    if (!spellChecker)
      return;
    spellChecker.destroy();
    document.properties.delete(SpellChecker.PROPERTY);
  }

  switch (action) {
    case 'add':
      installSpellChecker(document);
      break;
    case 'remove':
      uninstallSpellChecker(document);
      break;
   }
});
