// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @typedef {{
 *  lastUsedTime: !Date,
 *  promise: ?Promise,
 *  spelling: !Spelling,
 *  useCount: number,
 *  word: string
 * }}
 */
const SpellingResult = {};

(function() {
  /**
   * @enum {number}
   */
  const Spelling = {
    NONE: 0,
    CORRECT: 1,
    MISSPELLED: 2,
    DEBUG: 3 // green wave
  };

  /** @const @type {number} */
  const kMaxScanCount = 1000;

  /** @const @type {number} */
  const kMaxWordLength = 20;

  /** @const @type {number} */
  const kMinWordLength = 3;

  /** @const @type {string} */
  const kCheckerPropertyName = 'SpellChecker';

  /** @const @type {number} */
  const kMaxNumberOfRequests = 10;

  /** @const @type {number} */
  const kCheckIntervalMilliseconds = 200;

  /** @const @type {!Set.<string>} */
  const keywords = new Set();

  /**
   * @param {number} charCode
   * @return {boolean}
   */
  function isWordChar(charCode) {
    const category = Unicode.UCD[charCode].category;
    return category === 'Lu' || category === 'Ll';
  }

  //////////////////////////////////////////////////////////////////////
  //
  // Controller
  //
  class Controller {
    constructor() {
      /** @type {number} */
      this.numberOfRequests_ = 0;

      /**
       * @type {!Map.<string, SpellingResult>}
       *
       * Note: We expose |wordStateMap| for debugging and testing
       * purpose.
       */
      this.wordStateMap_ = new Map();
    }

    /**
     * @return {boolean}
     */
    canRequest() {
      return this.numberOfRequests_ < kMaxNumberOfRequests;
    }

    /**
     * @param {string} wordToCheck
     * @return {!Promise.<SpellingResult>}
     */
    requestCheckSpelling(wordToCheck) {
      const present = this.wordStateMap_.get(wordToCheck);
      if (present) {
        present.lastUsedTime = new Date();
        ++present.useCount;
        return present.promise ? present.promise : Promise.resolve(present);
      }

      const state = {
        lastUsedTime: new Date(),
        promise: null,
        useCount: 1,
        word: wordToCheck,
      };
      ++this.numberOfRequests_;
      this.wordStateMap_.set(wordToCheck, state);
      state.promise = Editor.checkSpelling(wordToCheck).then((isCorrect) => {
        --this.numberOfRequests_;
        state.lastUsedTime = new Date(),
        state.promise = null;
        state.spelling = isCorrect ? Spelling.CORRECT : Spelling.MISSPELLED;
        return state;
      });
      return state.promise;
    }
  }

  const controller = new Controller();

  //////////////////////////////////////////////////////////////////////
  //
  // PositionRunner
  //
  /** @class */
  class PositionRunner {
    /**
     * @param {!Document} document
     * @param {number} offset
     * @param {number} life
     */
    constructor(document, offset, life) {
      /** @const @type {!Document} */
      this.document_ = document;

      /** @type {number} */
      this.life_ = life;

      /** @const @type {number} */
      this.maxLife_ = life;

      /** @type {number} */
      this.offset_ = offset;
    }

    /** @return {boolean} */
    atEnd() { return this.offset_ === this.document_.length; }

    /** @return {boolean} */
    atStart() { return this.offset_ === 0; }

    /** @return {number} */
    charCode() { return this.document_.charCodeAt_(this.offset_); }

    /** @return {boolean} */
    isDead() { return this.life_ === 0 }

    moveNext() {
      if (this.isDead())
        return false;
      if (this.atEnd())
        return true;
      --this.life_;
      ++this.offset_;
      return true;
    }

    movePrevious() {
      if (this.isDead())
        return false;
      if (this.atStart())
        return true;
      --this.life_;
      --this.offset_;
      return true;
    }

    /** @return {boolean} */
    moveToEndOfWord() {
      while (!this.atEnd() && isWordChar(this.charCode())) {
        if (!this.moveNext())
          return false;
      }
      return true;
    }

    /** @return {boolean} */
    moveToNextWord() {
      while (!this.atEnd() && !isWordChar(this.charCode())) {
        if (!this.moveNext())
          return false;
      }
      return true;
    }

    /** @return {boolean} */
    moveToStartOfWord() {
      if (this.atStart())
        return true;
      if (this.atEnd() || !isWordChar(this.charCode()))
        while (!this.atStart()) {
          if (!this.movePrevious())
            return false;
          if (isWordChar(this.charCode()))
            break;
      }
      while (!this.atStart()) {
        if (!this.movePrevious())
          return false;
        if (!isWordChar(this.charCode()))
          return this.moveNext();
      }
      return true;
    }

    /** @return {number} */
    get offset() { return this.offset_; }

    /** @return {number} */
    get usedLife() { return this.maxLife_ - this.life_; }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // Scanner
  //
  class Scanner {
    /** @param {!Document} document */
    constructor(document) {
      /** @const @type {!Document} document */
      this.document_ = document;

      /** @type {boolean} */
      this.more_ = false;

      /** @type {number} */
      this.life_ = kMaxScanCount;
    }

    /** @return {boolean} */
    get more() { return this.more_; }

    /**
     * TODO(eval1749): Once closure compiler support class inside function,
     * we should annotate with |PositionRunner|.
     * @param {?} runner
     * @param {number} offset
     * @return {number}
     */
    needMore(runner, offset) {
      this.life_ -= runner.usedLife;
      this.more_ = true;
      return offset;
    }

    /**
     * TODO(eval1749): Once closure compiler support class inside function,
     * we should annotate with |SpellChecker|.
     * @param {?} spellChecker
     * @param {number} start
     * @param {number} end
     * @return {number}
     */
    scan(spellChecker, start, end) {
      let runner = new PositionRunner(this.document_, start, this.life_);
      if (!runner.moveToStartOfWord())
        return this.needMore(runner, start);

      while (runner.offset < end) {
        /** @type {number} */
        const wordStart = runner.offset;
        if (!runner.moveToEndOfWord())
          return this.needMore(runner, wordStart);

        /** @type {number} */
        const wordEnd = runner.offset;
        if (!spellChecker.checkSpelling(wordStart, wordEnd))
          return this.needMore(runner, wordStart);

        if (!runner.moveToNextWord())
          return this.needMore(runner, runner.offset);
      }

      this.life_ -= runner.usedLife;
      return end;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // SpellChecker
  //
  class SpellChecker {
    /**
     * Spell checker for specified document. Each document is associated to
     * separate |SpellChecker| instance.
     *
     * @param {!Document} document
     */
    constructor(document) {
      /** @type {TextWindow} */
      this.activeWindow_ = null;

      /** @type {number} */
      this.coldEnd = document.length;

      /** @type {number} */
      this.coldOffset = 0;

      /**
       * @const
       * @type {!Array.<!Range>}
       * List of ranges can be used for checking spelling.
       */
      this.freeRanges_ = new Array(10);
      for (var i = 0; i < this.freeRanges_.length; ++i)
        this.freeRanges_[i] = new Range(document);

      /** @type {number} */
      this.hotOffset = document.length;

      /** @const @type {!MutationObserver} */
      this.mutationObserver_ = new MutationObserver(
          this.mutationCallback.bind(this));

      /**
       * @const @type {!Range}
       * A Range object for scanning document.
       */
      this.range_ = new Range(document);

      /** @type {boolean} */
      this.shouldStopAtCaret_;

      /** @const @type {!OneShotTimer} */
      this.timer_ = new OneShotTimer();

      document.addEventListener(Event.Names.ATTACH,
                                this.didAttachWindow.bind(this));

      // Ignore document mutation during loading contents.
      document.addEventListener(Event.Names.BEFORELOAD,
                                this.beforeLoad.bind(this));

      // Restart spell checking from start of document after loading.
      document.addEventListener(Event.Names.LOAD, this.didLoad.bind(this));

      this.mutationObserver_.observe(document, {summary: true});
    }

    /**
     * @private
     */
    beforeLoad() {
      this.mutationObserver_.disconnect();
      this.timer_.stop();
    }

    /**
     * @param {number} wordStart
     * @param {number} wordEnd
     * @return {boolean}
     */
    checkSpelling(wordStart, wordEnd) {
      if (wordStart === wordEnd)
        return true;
      /** @const @type {!Range} */
      const range = this.range_;
      range.collapseTo(wordStart);
      range.end = wordEnd;
      range.setSpelling(Spelling.NONE);
      const length = wordEnd - wordStart;
      if (length < kMinWordLength ||
          length > kMaxWordLength) {
        return true;
      }

      if (this.shouldStopAtCaret_) {
        const selection = /** @type {!TextSelection} */(
            this.activeWindow_.selection);
        if (selection.focusOffset >= wordStart &&
            selection.focusOffset >= wordEnd) {
          // Since candidate word contains caret, we consider this word is still
          // changing.
          this.shouldStopAtCaret_ = false;
          return false;
        }
      }

      if (this.document.syntaxAt(wordStart) === 'keyword')
        return true;
      const word = range.text;
      if (keywords.has(word))
        return true;
      if (!this.freeRanges_.length || !controller.canRequest())
        return false;

      /** @type {!Range} */
      const markerRange = this.freeRanges_.pop();
      markerRange.collapseTo(wordStart);
      markerRange.end = wordEnd;
      controller.requestCheckSpelling(word).then((state) => {
        if (state.word === markerRange.text)
          this.markWord(markerRange, state.spelling);
        this.freeRanges_.push(markerRange);
      });
      return true;
    }

    // Cleanup resources used by spell checker.
    destroy() {
      this.timer_.stop();
    }

    /**
     * @private
     * @param {!UiEvent} event
     */
    didAttachWindow(event) {
      event.view.addEventListener(Event.Names.BLUR,
          this.didBlurWindow.bind(this));
      event.view.addEventListener(Event.Names.FOCUS,
          this.didFocusWindow.bind(this));
    }

    /**
     * @private
     * @param {!Event} event
     * Spell checker is stopped when window loses focus.
     */
    didBlurWindow(event) {
      this.activeWindow_ = null;
      this.timer_.stop();
    }

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
    // Note: To reduce misspelling in source code, e.g. function name, and
    // variable names, we don't check character syntax.
    //
    didFireTimer() {
      if (!this.activeWindow_)
        return;

      /** @const @type {!Document} */
      const document = this.document;

      // |didFireTimer| can be called before handling mutation callback, we should
      // make sure offsets don't exceed |document.length|.
      this.coldOffset = Math.min(this.coldOffset, this.document.length);
      this.coldEnd = Math.min(this.coldEnd, this.document.length);
      this.hotOffset = Math.min(this.hotOffset, this.document.length);

      /** @const @type {!Scanner} */
      const scanner = new Scanner(document);
      this.hotOffset = scanner.scan(this, this.hotOffset, document.length);
      this.coldOffset = scanner.scan(this, this.coldOffset, this.coldEnd);
      if (!scanner.more)
        return;
      this.startTimerIfNeeded();
    }

    /**
     * @private
     * Spell checking is started when window is focused.
     */
    didFocusWindow() {
      if (Window.focus instanceof TextWindow)
        this.activeWindow_ = Window.focus;
      this.startTimerIfNeeded();
    }

    /**
     * @private
     */
    didLoad() {
      this.coldOffset = 0;
      this.coldEnd = this.document.length;
      this.hotOffset = 0;
    }

    /** @return {!Document} */
    get document() { return this.range_.document; }

    /**
     * @param {!Range} wordRange
     * @param {Spelling} mark
     *
     * Mark first |word| without misspelled marker in document with |mark|.
     */
    markWord(wordRange, mark) {
      if (mark === Spelling.CORRECT)
        return;
      wordRange.setSpelling(mark);
    }

    /**
     * @param {!Array.<!MutationRecord>} mutations
     * @param {!MutationObserver} observer
     *
     * Resets hot offset to minimal changed offset and kicks word scanner.
     */
    mutationCallback(mutations, observer) {
      /** @type {!Range} */
      const range = this.range_;
      /** @type {!Document} */
      const document = range.document;
      /** @type {number} */
      const minOffset = mutations.reduce(function(previousValue, mutation) {
        return Math.min(previousValue, mutation.offset);
      }, document.length);
      this.coldOffset = Math.min(this.coldOffset, minOffset);
      this.coldEnd = Math.min(this.coldEnd, minOffset);
      this.hotOffset = Math.min(this.hotOffset, minOffset);
      this.shouldStopAtCaret_ = true;

      this.startTimerIfNeeded();
    }

    startTimerIfNeeded() {
      if (this.timer_.isRunning || !this.activeWindow_)
        return;
      this.timer_.start(kCheckIntervalMilliseconds,
                        this.didFireTimer.bind(this));
    }
  }

  // When document is created/destructed, we install/uninstall spell checker
  // to/from document.
  Document.addObserver(function(action, document) {
    /** @param {!Document} document */
    function installSpellChecker(document) {
      // TODO(eval1749): We should have generic way to disable spell checking for
      // document.
      if (document.name === '*javascript*')
        return;
      const spellChecker = new SpellChecker(document);
      document.properties.set(kCheckerPropertyName, spellChecker);
    }

    /** @param {!Document} document */
    function uninstallSpellChecker(document) {
      const spellChecker = document.properties.get(kCheckerPropertyName);
      if (!spellChecker)
        return;
      spellChecker.destroy();
      document.properties.delete(kCheckerPropertyName);
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

  Object.defineProperty(SpellChecker, 'keywords', {value: keywords});

  global.SpellChecker = SpellChecker;
  Object.freeze(SpellChecker);
  Object.freeze(SpellChecker.prototype);
})();
