// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @enum {number}
 */
global.Spelling = {
  NONE: 0,
  CORRECT: 1,
  MISSPELLED: 2
};

/**
 * Spell checker for specified document. Each document is associated to
 * separate |SpellChecker| instance.
 *
 * @constructor
 * @param {!Document} document
 */
var SpellChecker = function(document) {
  this.coldEnd = document.length;
  this.coldOffset = 0;
  this.freeRanges = new Array(SpellChecker.MAX_CHECKING);
  for (var i = 0; i < this.freeRanges.length; ++i) {
    this.freeRanges[i] = new Range(document);
  }
  this.hotOffset = document.length;
  this.mutationObserver = new MutationObserver(
      this.mutationCallback.bind(this));
  this.mutationObserver.observe(document, {summary: true});
  this.range = new Range(document);
  this.timer = new RepeatingTimer();

  var spellChecker = this;
  document.addEventListener(Event.Names.ATTACH, function(event) {
    event.view.addEventListener(Event.Names.BLUR,
        spellChecker.didBlurWindow.bind(spellChecker));
    event.view.addEventListener(Event.Names.FOCUS,
        spellChecker.didFocusWindow.bind(spellChecker));
  });
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
 *  busy: boolean,
 *  lastUsedTime: !Date,
 *  spelling: !Spelling,
 *  useCount: number,
 *  word: string
 * }}
 */
SpellChecker.SpellingResult

/** * @type {number} */
SpellChecker.num_checking;

/** @type {number} */
SpellChecker.prototype.coldEnd;

/** @type {number} */
SpellChecker.prototype.coldOffset;

/**
 * @type {!Array.<!Range>}
 * List of ranges can be used for checking spelling.
 */
SpellChecker.prototype.freeRanges;

/** @type {number} */
SpellChecker.prototype.hotOffset;

/** @type {!MutationObserver} */
SpellChecker.prototype.mutationObserver;

/**
 * @type {!Range}
 * A Range object for scanning document.
 */
SpellChecker.prototype.range;

/** @type {!RepeatingTimer} */
SpellChecker.prototype.timer;

/**
 * @param {!Range} word_range
 * Checks then mark word as correct or misspelled.
 */
SpellChecker.prototype.checkSpelling = function(word_range) {
  var spellChecker = this;
  if (!spellChecker.freeRanges.length ||
      SpellChecker.num_checking >= SpellChecker.MAX_CHECKING) {
    return false;
  }
  ++SpellChecker.num_checking;

  var marker_range = spellChecker.freeRanges.pop();
  marker_range.collapseTo(word_range.start);
  marker_range.end = word_range.end;
  var word_to_check = marker_range.text;
  SpellChecker.checkSpelling(word_to_check).then(function (result) {
    --SpellChecker.num_checking;
    if (result && !result.busy && result.word == marker_range.text)
      spellChecker.markWord(marker_range, result.spelling);
    spellChecker.freeRanges.push(marker_range);
  }).catch(console.log);
  return true;
};

// Cleanup resources used by spell checker.
SpellChecker.prototype.destroy = function() {
  this.timer.stop();
};

// Spell checker is stopped when window loses focus.
SpellChecker.prototype.didBlurWindow = function(event) {
  this.timer.stop();
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
  var range = this.range;
  var document = range.document;

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

  var hottest_offset = getHottestOffset();

  /**
   * @param {!Range} range
   * @return {boolean}
   */
  function isCheckableWord(range) {
    var length = range.end - range.start;
    if (length < SpellChecker.MIN_WORD_LENGTH ||
        length > SpellChecker.MAX_WORD_LENGTH) {
      return false;
    }
    if (hottest_offset >= range.start && hottest_offset <= range.end) {
      // The word is still changing, we ignore it.
      return false;
    }
    var category = Unicode.UCD[document.charCodeAt_(range.start)].category;
    // TODO(yosi) When we support spell checking other than English, we
    // should make other Unicode general categories are valid.
    return category == 'Lu' || category == 'Ll';
  }

  var num_checked = 0;

  /**
   * @param {!SpellChecker} spellChecker
   * @param {number} start
   * @param {number} end
   * @return {number} Text offset for next scanning.
   */
  function scan(spellChecker, start, end) {
    if (SpellChecker.num_checking >= SpellChecker.MAX_CHECKING)
      return start;
    range.collapseTo(start);
    range.startOf(Unit.WORD);
    while (num_checked < SpellChecker.CHECK_INTERVAL_LIMIT &&
           range.start < end && spellChecker.freeRanges.length) {
      var restart_offset = range.start;
      range.endOf(Unit.WORD, Alter.EXTEND);
      range.setSpelling(Spelling.NONE);
      if (range.end == document.length) {
        // Word seems not to be completed yet. Spell checker will sleep
        // until document is changed.
        num_checked += range.end - range.start;
        return range.end;
      }

      // To reduce garbage collection, we check word in |word_range| is whether
      // suitable for checking spelling or not.
      if (isCheckableWord(range) && !spellChecker.checkSpelling(range))
        return restart_offset;

      num_checked += range.end - range.start;

      // Move to next word.
      // <#>include     => #|include
      // foo<;>\n   bar => foo;\n  |bar
      var word_start = range.start;
      range.collapseTo(word_start);
      range.move(Unit.WORD);
    }
    return range.start;
  }

  if (this.hotOffset < document.length)
    this.hotOffset = scan(this, this.hotOffset, document.length);
  this.coldOffset = scan(this, this.coldOffset, this.coldEnd);

  var rest = document.length - this.hotOffset + this.coldEnd - this.coldOffset;
  if (rest > 0)
    return;
  this.timer.stop();
};

// Spell checking is started when window is focused.
SpellChecker.prototype.didFocusWindow = function() {
  this.startTimeIfNeeded();
};

/**
 * @param {!Range} word_range
 * @param {Spelling} mark
 *
 * Mark first |word| without misspelled marker in document with |mark|.
 */
SpellChecker.prototype.markWord = function(word_range, mark) {
  if (mark == Spelling.CORRECT)
    return;
  // TODO(yosi) We should not mark for word in source code.
  word_range.setSpelling(mark);
};

/**
 * @this {!SpellChecker}
 * @param {!Array.<!MutationRecord>} mutations
 * @param {!MutationObserver} observer
 *
 * Resets hot offset to minimal changed offset and kicks word scanner.
 */
SpellChecker.prototype.mutationCallback = function(mutations, observer) {
  var range = this.range;
  var document = range.document;
  /** @type {number} */
  var min_offset = mutations.reduce(function(previousValue, mutation) {
    return Math.min(previousValue, mutation.offset);
  }, document.length);
  this.coldOffset = Math.min(this.coldOffset, min_offset);
  this.coldEnd = min_offset;
  this.hotOffset = min_offset;

  var has_focus = document.listWindows().some(function(window) {
    return Window.focus == window;
  });
  if (has_focus)
    this.startTimeIfNeeded();
};

SpellChecker.prototype.startTimeIfNeeded = function() {
  if (this.timer.isRunning)
    return;
  this.timer.start(SpellChecker.CHECK_INTERVAL_MS, this.didFireTimer, this);
};

/**
 * @type {!Map.<string, SpellChecker.SpellingResult>}
 *
 * Note: We expose |SpellChecker.wordStateMap| for debugging and testing
 * purpose.
 */
SpellChecker.wordStateMap = new Map();

/**
 * @param {string} word_to_check
 * @return {!Promise.<SpellChecker.SpellingResult>}
 */
SpellChecker.checkSpelling = function(word_to_check) {
  if (!SpellChecker.RE_WORD.test(word_to_check))
    return Promise.resolve(null);
  var result = SpellChecker.wordStateMap.get(word_to_check);
  if (result) {
    result.lastUsedTime = new Date();
    ++result.useCount;
    return Promise.resolve(result);
  }
  var state = {
    busy: true,
    lastUsedTime: new Date(),
    useCount: 1,
    word: word_to_check,
  };
  SpellChecker.wordStateMap.set(word_to_check, state);
  var promise = Editor.checkSpelling(word_to_check).then(
      function(is_correct) {
        state.busy = false;
        state.lastUsedTime = new Date(),
        state.spelling = is_correct ? Spelling.CORRECT : Spelling.MISSPELLED;
        return state;
      });
  return /** @type {!Promise.<SpellChecker.SpellingResult>} */(promise);
};

// When document is created/destructed, we install/uninstall spell checker
// to/from document.
Document.addObserver(function(action, document) {
  /** @param {!Document} document */
  function installSpellChecker(document) {
    // TODO(yosi) We should have generic way to disable spell checking for
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
