// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function debug(...params) {
}

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
  const kMaxColdScanCount = 1000 * 2;

  /** @const @type {number} */
  const kMaxHotScanCount = 400;

  /** @const @type {number} */
  const kMaxWordLength = 20;

  /** @const @type {number} */
  const kMinWordLength = 3;

  /** @const @type {number} */
  const kMaxNumberOfRequests = 100;

  /** @const @type {number} */
  const kHotScanIntervalMs = 100;

  /** @const @type {!RegExp} */
  const RE_WORD = new RegExp('^[A-Za-z][a-z]{' +
    (kMinWordLength - 1) + ',' +
    (kMaxWordLength - 1) + '}$');

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

    /**
     * @param {!Range} wordRange
     * @param {Spelling} mark
     *
     * Mark first |word| without misspelled marker in document with |mark|.
     */
  function markWord(wordRange, mark) {
    if (mark === Spelling.CORRECT)
      return;
    wordRange.setSpelling(mark);
  }

  //////////////////////////////////////////////////////////////////////
  //
  // HotScehduler
  //
  class HotScehduler {
    constructor() {
      this.task_ = null;

      /** @const @type {!OneShotTimer} */
      this.timer_ = new OneShotTimer();
    }

    didFireTimer() {
      if (!this.task_)
        return;
      this.task_.run();
    }

    remove(task) {
      if (this.task_ !== task)
        return;
      this.task_ = null;
    }

    schedule(task) {
      this.task_ = task;
      if (this.timer_.isRunning)
        return;
      this.timer_.start(kHotScanIntervalMs, this.didFireTimer, this);
    }
  }
  const hotScheduler = new HotScehduler();

  //////////////////////////////////////////////////////////////////////
  //
  // Controller
  //
  class Controller {
    constructor() {
      /** @type {number} */
      this.numberOfRequests_ = 0;

      /** @const @type {!Set.<string>} */
      this.correctWords_ = new Set();

      /** @const @type {!Set.<string>} */
      this.missSpelledWords_ = new Set();
    }

    /**
     * @return {boolean}
     */
    canRequest() {
      return this.numberOfRequests_ < kMaxNumberOfRequests;
    }

    /**
     * @param {string}word
     * @return {Spelling}
     */
    checkSpelling(word) {
      if (!RE_WORD.test(word) || keywords.has(word) ||
          this.correctWords_.has(word)) {
        return Spelling.CORRECT;
      }
      if (this.missSpelledWords_.has(word))
        return Spelling.MISSPELLED;
      return Spelling.NONE;
    }

    /**
     * TODO(eval1749): We should use |Promise.<boolean>| for return type
     * once Blink IDL parser supports Promise type.
     * @param {string} word
     * @return {!Promise.<?>}
     */
    requestCheckSpelling(word) {
      debug('requestCheckSpelling', word);
      ++this.numberOfRequests_;
      return Editor.checkSpelling(word).then((isCorrect) => {
        --this.numberOfRequests_;
        if (isCorrect)
          this.correctWords_.add(word);
        else
          this.missSpelledWords_.add(word);
        return isCorrect;
      });
    }
  }
  const controller = new Controller();

  /**
   * @class
   * @implements {Runnable}
   */
  class Scanner {
    /**
     * @param {!Range} range
     * @param {number} start
     * @param {number} end
     * @param {number} life
     */
    constructor(range, start, end, life) {
      /** @const @type {!Document} */
      this.document_ = range.document;

      /** @type {number} */
      this.end_ = end;

      /** @type {number} */
      this.life_ = life;

      /** @type {number} */
      this.offset_ = start;

      /** @type {!Range} */
      this.range_ = range;

      /** @type {number} */
      this.start_ = start;
    }

    /** @return {boolean} */
    atEnd() { return this.offset_ >= this.document_.length; }

    /** @return {boolean} */
    atStart() { return this.offset_ === 0; }

    /** @return {number} */
    charCode() { return this.document_.charCodeAt_(this.offset_); }

    /** @return {number} */
    get end() { return this.end_; }

    /** @return {number} */
    get life() { return this.life_; }

    /** @return {number} */
    get offset() { return this.offset_; }

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

    /** @param {number} start */
    removeMarker(start) {
      if (start === this.offset_)
        return;
      this.range_.collapseTo(start);
      this.range_.end = this.offset_;
      this.range_.setSpelling(Spelling.NONE);
    }

    /**
     * @param {number} wordStart
     * @param {number} wordEnd
     * @return {string}
     */
    prepareRequest(wordStart, wordEnd) {
      if (wordStart === wordEnd)
        return '';

      /** @const @type {number} */
      const length = wordEnd - wordStart;
      if (length < kMinWordLength || length > kMaxWordLength)
        return '';

      if (this.document_.syntaxAt(wordStart) === 'keyword')
        return '';

      /** @const @type {!Range} */
      const range = this.range_;
      range.collapseTo(wordStart);
      range.end = wordEnd;

      /** @type {string} */
      const word = range.text;

      /** @type {Spelling} */
      const spelling = controller.checkSpelling(word);
      if (spelling !== Spelling.NONE) {
        range.setSpelling(spelling);
        return '';
      }
      return word;
    }

    /**
     * @param {number} newOffset
     * @param {number} newEnd
     * @param {number} newLife
     */
    reset(newOffset, newEnd, newLife) {
      this.offset_ = Math.min(this.offset_, newOffset);
      this.end_ = newEnd;
      this.life_ = newLife;
    }
  }

  /**
   * TODO(eval1749): Once closure compiler support class inside function,
   * we should annotate with |SpellChecker|.
   */
  function* scannerWords() {
    /** @type {number} */
    const maxOffset = this.document_.length;
    this.end_ = Math.min(this.end_, maxOffset);
    this.offset_ = Math.min(this.offset_, maxOffset);

    /** @type {number} */
    let lastOffset = this.offset_;
    if (!this.moveToStartOfWord()) {
      this.removeMarker(lastOffset);
      return;
    }

    while (this.offset_ < this.end_) {
      this.removeMarker(lastOffset);

      /** @type {number} */
      const wordStart = this.offset_;
      if (!this.moveToEndOfWord())
        break;

      /** @type {number} */
      const wordEnd = this.offset_;
      if (wordStart !== wordEnd) {
        this.removeMarker(wordStart);
        lastOffset = this.offset_;
        yield {start: wordStart, end: lastOffset};
      }

      if (!this.moveToNextWord())
        break;
    }
    this.removeMarker(lastOffset);
  }

  /** @type {function():!Iterator.<{start: number, end: number}>} */
  Scanner.prototype.words;
  Object.defineProperty(Scanner.prototype, 'words', {value: scannerWords});

  //////////////////////////////////////////////////////////////////////
  //
  // ColdPainter
  //
  class ColdPainter extends Scanner {
    /** @param {!Range} range */
    constructor(range) {
      super(range, 0, 0, 0);
    }

    run() {
      const range = this.range_;
      for (let wordRange of this.words()) {
        range.collapseTo(wordRange.start);
        range.end = wordRange.end;
        const spelling = controller.checkSpelling(range.text);
        range.setSpelling(spelling);
      }
      this.schedule();
    }

    schedule() {
      if (this.offset_ >= this.end_)
        return;
      this.life_ = kMaxColdScanCount;
      taskScheduler.schedule(this);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // ColdScanner
  //
  class ColdScanner extends Scanner {
    /** @param {!Range} range */
    constructor(range) {
      super(range, 0, 0, 0);

      /** @type {number} */
      this.checked_ = 0;

      // TODO(eval1749): We should use |Promise.<boolean>| for return type
      // once Blink IDL parser supports Promise type.
      this.painter_ = new ColdPainter(range);
    }

    /**
     * @param {number} wordStart
     * @param {number} wordEnd
     * @return {boolean}
     */
    checkWord(wordStart, wordEnd) {
      const word = this.prepareRequest(wordStart, wordEnd);
      if (word.length === 0) {
        this.checked_ = wordEnd;
        return true;
      }

      if (!controller.canRequest())
        return false;

      controller.requestCheckSpelling(word).then(() => {
        this.checked_ = wordEnd;
      });
      return true;
    }

    run() {
      debug('taskScheduler.run', this.offset_, this.end_);
      for (let wordRange of this.words()) {
        if (!this.checkWord(wordRange.start, wordRange.end))
          break;
      }
      this.painter_.reset(this.checked_, this.checked_, this.life);
      this.painter_.run();
      this.schedule();
    }

    schedule() {
      if (this.offset_ >= this.end_)
        return;
      this.life_ = kMaxColdScanCount;
      taskScheduler.schedule(this);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // HotScanner
  //
  class HotScanner extends Scanner {
    /** @param {!Range} range */
    constructor(range) {
      super(range, 0, 0, 0);

      /** @type {TextSelection} */
      this.activeSelection_ = null;

      /**
       * @const
       * @type {!Array.<!Range>}
       * List of ranges can be used for checking spelling.
       */
      this.freeRanges_ = new Array(10);
      for (var i = 0; i < this.freeRanges_.length; ++i)
        this.freeRanges_[i] = new Range(range.document);
    }

    /**
     * @private
     * @return {number} A active caret offset
     */
    activeCaretOffset() {
      if (!this.activeSelection_)
        return -1;
      return this.activeSelection_.focusOffset;
    }

    /**
     * @param {number} wordStart
     * @param {number} wordEnd
     * @return {boolean}
     */
    checkWord(wordStart, wordEnd) {
      const word = this.prepareRequest(wordStart, wordEnd);
      if (word.length === 0)
        return true;

      if (!this.freeRanges_.length || !controller.canRequest())
        return false;

      /** @type {!Range} */
      const markerRange = this.freeRanges_.pop();
      markerRange.collapseTo(wordStart);
      markerRange.end = wordEnd;
      controller.requestCheckSpelling(word).then((isCorrect) => {
        this.schedule();
        this.freeRanges_.push(markerRange);
        if (word !== markerRange.text)
          return;
        const marker = isCorrect ? Spelling.CORRECT : Spelling.MISSPELLED;
        markWord(markerRange, marker);
      });
      return true;
    }

    didBlurWindow() {
      this.activeSelection_ = null;
    }

    didFocusWindow() {
      if (Window.focus instanceof TextWindow) {
        this.activeSelection_ =
            /** @type {!TextSelection} */(Window.focus.selection);
        return;
      }
      this.activeSelection_ = null;
    }

    run() {
      /** @type {number} */
      const caretOffset = this.activeCaretOffset();

      debug('hotScanner.run', this.offset_, this.end_, this.life_);

      for (let wordRange of this.words()) {
        if (caretOffset >= wordRange.start && caretOffset <= wordRange.end) {
          // Since candidate word contains caret, we consider this word is still
          // changing.
          debug('hot word', wordRange.start, wordRange.end,
                      this.range_.text);
          this.offset_ = wordRange.start;
          break;
        }
        if (!this.checkWord(wordRange.start, wordRange.end))
          break;
      }
    }

    schedule() {
      if (this.offset_ >= this.end_)
        return;
      this.life_ = kMaxHotScanCount;
      hotScheduler.schedule(this);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // SpellChecker
  //
  //
  //  Check spelling in hot region and cold region. When we check all words in
  //  cold region, |coldOffset| >= |coldEnd|, we check all the words in the
  //  document.
  // 
  //  Cold and hot regions are updated in mutation observer callback by using
  //  minimum changed offset.
  // 
  //    // Cold region: |coldOffset| to |coldEnd|.
  //    // Hot region: |hotOffset| to end of document.
  // 
  // 
  //          |...........|....................|
  //   offset 0           minimum change       document.length
  //          === cold === ======= hot ========
  // 
  //  Word scanner, |scan()|, stops hot word which contains caret not to check
  //  spell incomplete word.
  // 
  //  Note: To reduce misspelling in source code, e.g. function name, and
  //  variable names, we don't check character syntax.
  class SpellChecker {
    /**
     * Spell checker for specified document. Each document is associated to
     * separate |SpellChecker| instance.
     *
     * @param {!Document} document
     */
    constructor(document) {
      /** @type {!Document} */
      this.document_ = document;

      /** @type {!Range} */
      const range = new Range(document);

      /** @type {?} TODO(eval1749): We should use |ColdScanner| here. */
      this.coldScanner_ = new ColdScanner(range);

      /** @type {?} TODO(eval1749): We should use |HotScanner| here. */
      this.hotScanner_ = new HotScanner(range);

      /** @const @type {!MutationObserver} */
      this.mutationObserver_ = new MutationObserver(
          this.mutationCallback.bind(this));

      document.addEventListener(Event.Names.ATTACH,
                                this.didAttachWindow.bind(this));

      // Ignore document mutation during loading contents.
      document.addEventListener(Event.Names.BEFORELOAD,
                                this.beforeLoad.bind(this));

      // Restart spell checking from start of document after loading.
      document.addEventListener(Event.Names.LOAD, this.didLoad.bind(this));

      this.mutationObserver_.observe(document, {summary: true});
      this.restartColdScanner();
    }

    /**
     * @private
     */
    beforeLoad() {
      debug('beforeLoad', this.document_);
      taskScheduler.remove(this.coldScanner_);
      hotScheduler.remove(this.hotScanner_);
    }

    // Cleanup resources used by spell checker.
    destroy() {
      taskScheduler.remove(this.coldScanner_);
      hotScheduler.remove(this.hotScanner_);
    }

    /**
     * @private
     * @param {!UiEvent} event
     */
    didAttachWindow(event) {
      const window = /** @type {!TextWindow} */(event.view);
      window.addEventListener(Event.Names.BLUR,
          this.didBlurWindow.bind(this));
      window.addEventListener(Event.Names.FOCUS,
          this.didFocusWindow.bind(this));
      window.addEventListener(Event.Names.SELECTIONCHANGE,
          this.didChangeSelection.bind(this));
      this.hotScanner_.didFocusWindow();
      this.coldScanner_.schedule();
    }

    /**
     * @private
     * @param {!Event} event
     * Spell checker is stopped when window loses focus.
     */
    didBlurWindow(event) {
      this.hotScanner_.didBlurWindow();
    }

    /** @private */
    didChangeSelection() {
      this.hotScanner_.schedule();
    }

    /**
     * @private
     * Spell checking is started when window is focused.
     */
    didFocusWindow() {
      this.hotScanner_.didFocusWindow();
    }

    /**
     * @private
     */
    didLoad() {
      this.restartColdScanner();
    }

    /**
     * @param {!Array.<!MutationRecord>} mutations
     * @param {!MutationObserver} observer
     *
     * Resets hot offset to minimal changed offset and kicks word scanner.
     */
    mutationCallback(mutations, observer) {
      /** @type {!Document} */
      const document = this.document_;
      /** @type {number} */
      const minOffset = mutations.reduce((previousValue, mutation) => {
        return Math.min(previousValue, mutation.offset);
      }, document.length);
      this.coldScanner_.reset(minOffset, minOffset, kMaxColdScanCount);
      this.hotScanner_.reset(minOffset, document.length, kMaxHotScanCount);
      this.hotScanner_.run();
    }

    /** @private */
    restartColdScanner() {
      this.coldScanner_.reset(0, this.document_.length, kMaxColdScanCount);
      taskScheduler.schedule(this.coldScanner_);
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
      document.properties.set(SpellChecker.name, spellChecker);
    }

    /** @param {!Document} document */
    function uninstallSpellChecker(document) {
      const spellChecker = document.properties.get(SpellChecker.name);
      if (!spellChecker)
        return;
      spellChecker.destroy();
      document.properties.delete(SpellChecker.name);
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
