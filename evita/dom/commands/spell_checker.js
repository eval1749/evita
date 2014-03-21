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
    event.view.addEventListener('selectionchange',
        spellChecker.didChangeSelection.bind(spellChecker));
  });
}

/** @const @type {number} */
SpellChecker.CHECK_INTERVAL_LIMIT = 100;

/** @const @type {number} */
SpellChecker.CHECK_INTERVAL_MS = 200;

SpellChecker.MAX_WORD_LENGTH = 20;
SpellChecker.MIN_WORD_LENGTH = 3;

/** @const @type {string} */
SpellChecker.PROPERTY = 'SpellChecker';

/** @const @type {!RegExp} */
SpellChecker.RE_WORD = new RegExp('^[A-Za-z][a-z]{' +
    (SpellChecker.MIN_WORD_LENGTH - 1) + ',' +
    (SpellChecker.MAX_WORD_LENGTH - 1) + '}$');

/**
 * @enum {string}
 */

/**
 * @typedef{{
 *  busy: boolean,
 *  lastUsedTime: !Date,
 *  spelling: !Spelling,
 *  word: string
 * }}
 */
SpellChecker.SpellingResult

/** @type {number} */
SpellChecker.prototype.coldEnd;

/** @type {number} */
SpellChecker.prototype.coldOffset;

/** @type {number} */
SpellChecker.prototype.hotOffset;

/** @type {!MutationObserver} */
SpellChecker.prototype.mutationObserver;

/** @type {!Range} */
SpellChecker.prototype.range;

/** @type {!RepeatingTimer} */
SpellChecker.prototype.timer;

SpellChecker.prototype.checkSpelling = function(word_to_check) {
  var spellChecker = this;
  SpellChecker.checkSpelling(word_to_check).then(function (result) {
    if (!result || result.busy)
      return;
    spellChecker.markWord(result.word, result.spelling);
  }).catch(console.log);
};

SpellChecker.prototype.destroy = function() {
  this.timer.stop();
};

SpellChecker.prototype.didBlurWindow = function(event) {
  this.timer.stop();
};

SpellChecker.prototype.didChangeSelection = function(event) {
};

// Check spelling in hot range and cold range.
//        |...........|...................|
// offset 0           minimum change      document.length
//            cold          hot
//
// To reduce misspelling in source code, e.g. function name, and variable
// names, we don't check character syntax.
//
SpellChecker.prototype.didFireTimer = function() {
  var range = this.range;
  var document = range.document;

  function getHottestOffset() {
    if (!(Window.focus instanceof TextWindow))
      return -1;
    if (Window.focus.document != document)
      return -1;
    var range = Window.focus.selection.range;
    return range.collapsed ? range.start : -1;
  }

  var checked = 0;
  var hottest_offset = getHottestOffset();

  /**
   * @param {!SpellChecker} spellChecker
   * @param {number} start
   * @param {number} end
   */
  function scan(spellChecker, start, end) {
    range.collapseTo(start);
    range.startOf(Unit.WORD);
    while (checked < SpellChecker.CHECK_INTERVAL_LIMIT && range.start < end) {
      range.endOf(Unit.WORD, Alter.EXTEND);
      range.setSpelling(Spelling.NONE);
      if (hottest_offset >= range.start && hottest_offset <= range.end) {
        // The word is still changing, we ignore it, e.g. |enum|, |instanceof|.
        break;
      }
      checked += range.end - range.start;
      if (range.end == document.length) {
        // Word seems not to be completed yet.
        break;
      }
      var length = range.end - range.start;
      if (length >= SpellChecker.MIN_WORD_LENGTH &&
          length <= SpellChecker.MAX_WORD_LENGTH) {
        spellChecker.checkSpelling(range.text);
      }

      // Move to next word.
      // <#>include     => #|include
      // foo<;>\n   bar => foo;\n  |bar
      var word_start = range.start;
      range.collapseTo(word_start);
      range.move(Unit.WORD);
      checked += range.end - word_start;
    }
    return range.start;
  }

  if (this.hotOffset < document.length)
    this.hotOffset = scan(this, this.hotOffset, document.length);

  this.coldOffset = scan(this, this.coldOffset, this.coldEnd);
  if (this.hotOffset == document.length && this.coldOffset >= this.coldEnd)
    this.timer.stop();
};

SpellChecker.prototype.didFocusWindow = function() {
  this.startTimeIfNeeded();
};

/**
 * Mark all |word| in document with |mark|.
 * @param {string} word
 * @param {Spelling} mark
 */
SpellChecker.prototype.markWord = function(word, mark) {
  var regex = new Editor.RegExp(word, {
    ignoreCase: false,
    matchExact: true,
    matchWord: true,
  });
  var range = this.range;
  var document = range.document;
  var runner = 0;
  for (;;) {
    var matches = document.match_(regex, runner, document.length);
    if (!matches)
      break;
    var match = matches[0];
    range.collapseTo(match.start);
    range.end = match.end;
    // TODO(yosi) We should not mark for word in source code.
    range.setSpelling(mark);
    runner = match.end;
  }
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

  /** @type {!Map.<string, SpellChecker.SpellingResult>} */
  var wordStateMap = new Map();

/**
 * @param {string} word_to_check
 * @return {!Promise.<SpellChecker.SpellingResult>}
 */
SpellChecker.checkSpelling = (function() {
  /**
   * @param {string} word_to_check
   * @return {!Promise.<SpellChecker.SpellingResult>}
   */
  return function(word_to_check) {
    if (!SpellChecker.RE_WORD.test(word_to_check))
      return Promise.cast(null);
    var result = wordStateMap.get(word_to_check);
    if (result) {
      result.lastUsedTime = new Date();
      return Promise.cast(result);
    }
    var state = {
      busy: true,
      lastUsedTime: new Date(),
      word: word_to_check,
    };
    wordStateMap.set(word_to_check, state);
    var promise = Editor.checkSpelling(word_to_check).then(
        function(is_correct) {
          state.busy = false;
          state.lastUsedTime = new Date(),
          state.spelling = is_correct ? Spelling.CORRECT : Spelling.MISSPELLED;
          return state;
        });
    return /** @type {!Promise.<SpellChecker.SpellingResult>} */(promise);
  };
})();

// Install/uninstall spell checker to/from document.
Document.addObserver(function(action, document) {
  /** @param {!Document} document */
  function installSpellChecker(document) {
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
