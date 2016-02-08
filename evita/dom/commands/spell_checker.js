// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  const kColdScanStartDelay = 100;

  /** @const @type {number} */
  const kMaxColdScanCount = 1000 * 4;

  /** @const @type {number} */
  const kMaxHotScanCount = 400;

  /** @const @type {number} */
  const kMaxWordLength = 20;

  /** @const @type {number} */
  const kMinWordLength = 3;

  /** @const @type {number} */
  const kMaxNumberOfRequests = 100;

  /** @const @type {number} */
  const kHotScanStartDelay = 100;

  /** @const @type {!RegExp} */
  const RE_WORD = new RegExp('^[A-Za-z][a-z]{' +
    (kMinWordLength - 1) + ',' +
    (kMaxWordLength - 1) + '}$');

  /** @const @type {!Set.<string>} */
  const keywords = new Set();

  /**
   * @param {!Spelling} spelling
   * @return {!Spelling}
   */
  function colorOf(spelling) {
    return spelling == Spelling.MISSPELLED ? spelling : Spelling.NONE;
  }

  /**
   * @param {!TextDocument} document
   * @param {number} start
   * @param {number} end
   * @param {!Spelling} spelling
   */
  function paint(document, start, end, spelling) {
    document.setSpelling(start, end, colorOf(spelling));
  }

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
     * @param {!TextDocument} document
     * @param {number} start
     * @param {number} end
     * @param {number} life
     */
    constructor(document, start, end, life) {
      /** @private @const @type {!TextDocument} */
      this.document_ = document;

      /** @private @type {number} */
      this.end_ = end;

      /** @private @type {number} */
      this.life_ = life;

      /** @private @type {number} */
      this.offset_ = start;
    }

    /** @return {boolean} */
    atEnd() { return this.offset_ >= this.document_.length; }

    /** @return {boolean} */
    atStart() { return this.offset_ === 0; }

    /** @return {boolean} */
    atWordChar() {
      return Unicode.UCD[this.charCode()].category.charCodeAt(0) === 0x4C;
    }

    /** @return {number} */
    charCode() { return this.document_.charCodeAt(this.offset_); }

    /** @return {!TextDocument} */
    get document() { return this.document_; }

    /** @return {number} */
    get end() { return this.end_; }

    /** @return {number} */
    get life() { return this.life_; }

    /** @return {number} */
    get offset() { return this.offset_; }

    /**
     * Tasks invoked by scheduler may access content of document before mutation
     * observer callback, we should ensure |offset_| and |end_| to hold valid
     * offsets.
     * 
     */
    ensureOffsets() {
      this.offset_ = Math.min(this.offset_, this.document_.length);
      this.end_ = Math.min(this.end_, this.document_.length);
    }

    /** @return {boolean} */
    isDead() { return this.life_ === 0 }

    /**
     * @param {!Unicode.CharacterData} wordData
     * @return {boolean}
     *
     * Note: The script for U+30FC, Katakana-Hiragana prolonged sound mark, is
     * Unicode.Script.COMMON, instead of "HIRAGANA" or KATAKANA.
     */
    isWordChar(wordData) {
      const data = Unicode.UCD[this.charCode()];
      if (data.category.charCodeAt(0) !== 0x4C)
        return false;
      return wordData.script === data.script;
    }

    moveNext() {
      if (this.isDead() || this.atEnd())
        return false;
      --this.life_;
      ++this.offset_;
      return true;
    }

    movePrevious() {
      if (this.isDead() || this.atStart())
        return false;
      --this.life_;
      --this.offset_;
      return true;
    }

    /** @return {boolean} */
    moveToEndOfWord() {
      if (this.atEnd())
        return false;
      const wordData = Unicode.UCD[this.charCode()];
      if (wordData.category.charCodeAt(0) !== 0x4C)
        return true;
      while (!this.atEnd() && this.isWordChar(wordData)) {
        if (!this.moveNext())
          return false;
      }
      return true;
    }

    /** @return {boolean} */
    moveToNextWord() {
      while (!this.atEnd() && !this.atWordChar()) {
        if (!this.moveNext())
          return false;
      }
      return true;
    }

    /** @return {boolean} */
    moveToStartOfWord() {
      if (this.atStart())
        return true;
      if (this.atEnd() || !this.atWordChar()) {
        while (!this.atStart()) {
          if (!this.movePrevious())
            return false;
          if (this.atWordChar())
            break;
        }
      }
      if (this.atStart())
        return true;
      const wordData = Unicode.UCD[this.charCode()];
      while (!this.atStart()) {
        if (!this.movePrevious())
          return false;
        if (!this.isWordChar(wordData))
          return this.moveNext();
      }
      return true;
    }

    /** @param {number} start */
    removeMarker(start) {
      if (start === this.offset_)
        return;
      paint(this.document_, start, this.offset_, Spelling.NONE);
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

      /** @type {string} */
      const word = this.document_.slice(wordStart, wordEnd);

      /** @type {Spelling} */
      const spelling = controller.checkSpelling(word);
      if (spelling !== Spelling.NONE) {
        paint(this.document_, wordStart, wordEnd, spelling);
        return '';
      }
      return word;
    }

    /**
     * @protected
     * @param {number} newLife
     */
    resetLife(newLife) {
      this.life_ = newLife;
    }

    /**
     * @param {number} newStart
     * @param {number} newEnd
     */
    resetOffset(newStart, newEnd) {
      this.offset_ = newStart;
      this.end_ = newEnd;
    }

    /**
     * @param {number} newOffset
     * @param {number} newEnd
     */
    updateOffset(newOffset, newEnd) {
      console.assert(newOffset <= newEnd);
      this.offset_ = Math.min(newOffset, this.offset_);
      this.end_ = newEnd;
    }

    /**
     * @return {!Generator<{start: number, end: number}>}
     * Generates a word range starting from |this.offset_| until end of
     * document.
     */
    *words() {
      if (!this.moveToStartOfWord())
        return;
      for (;;) {
        /** @type {number} */
        const wordStart = this.offset_;
        if (!this.moveToEndOfWord())
          return;

        if (wordStart !== this.offset_)
          yield {start: wordStart, end: this.offset_};

        if (!this.moveToNextWord())
          return;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // ColdPainter
  //
  class ColdPainter extends Scanner {
    /** @param {!TextDocument} document */
    constructor(document) {
      super(document, 0, 0, 0);
    }

    run() {
      this.ensureOffsets();
      for (let wordRange of this.words()) {
        const word = this.document.slice(wordRange.start, wordRange.end);
        const spelling = controller.checkSpelling(word);
        paint(this.document, wordRange.start, wordRange.end, spelling);
      }
      this.schedule();
    }

    schedule() {
      if (this.offset_ >= this.end_)
        return;
      this.resetLife(kMaxColdScanCount);
      taskScheduler.schedule(this);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // ColdScanner
  //
  class ColdScanner extends Scanner {
    /** @param {!TextDocument} document */
    constructor(document) {
      super(document, 0, 0, 0);

      /** @type {number} */
      this.checked_ = 0;

      // TODO(eval1749): We should use |Promise.<boolean>| for return type
      // once Blink IDL parser supports Promise type.
      this.painter_ = new ColdPainter(document);
    }

    /**
     * @private
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
      return false;
    }

    /**
     * @param {number} hotStart
     */
    didChangeTextDocument(hotStart) {
      this.updateOffset(hotStart, hotStart);
      this.schedule(kColdScanStartDelay);
    }

    /*
     * Scan and paint on whole document.
     */
    didLoadTextDocument() {
      this.resetOffset(0, this.document.length);
      this.schedule(0);
    }

    run() {
      this.ensureOffsets();
      this.resetLife(kMaxColdScanCount);
      for (let wordRange of this.words()) {
        if (!this.checkWord(wordRange.start, wordRange.end))
          break;
      }
      this.painter_.resetOffset(this.offset, this.checked_);
      this.resetLife(this.life);
      this.painter_.run();
      this.schedule(0);
    }

    /**
     * @private
     * @param {number} delay
     */
    schedule(delay) {
      if (this.offset_ >= this.end_)
        return;
      taskScheduler.schedule(this, delay);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // HotScanner
  //
  class HotScanner extends Scanner {
    /** @param {!TextRange} range */
    constructor(range) {
      super(range.document, 0, 0, 0);

      /** @type {TextSelection} */
      this.activeSelection_ = null;

      /** @type {boolean} */
      this.caretIsHot_ = false;

      /**
       * @const
       * @type {!Array.<!TextRange>}
       * List of ranges can be used for checking spelling.
       */
      this.freeRanges_ = new Array(10);
      for (var i = 0; i < this.freeRanges_.length; ++i)
        this.freeRanges_[i] = new TextRange(range.document);
    }

    /**
     * @private
     * @return {number} A active caret offset
     */
    activeCaretOffset() {
      if (!this.caretIsHot_ || !this.activeSelection_)
        return -1;
      return this.activeSelection_.focusOffset;
    }

    /**
     * @private
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

      /** @type {!TextRange} */
      const markerRange = this.freeRanges_.pop();
      markerRange.collapseTo(wordStart);
      markerRange.end = wordEnd;
      controller.requestCheckSpelling(word).then((isCorrect) => {
        this.schedule(0);
        this.freeRanges_.push(markerRange);
        if (word !== markerRange.text)
          return;
        const spelling = isCorrect ? Spelling.CORRECT : Spelling.MISSPELLED;
        markerRange.setSpelling(colorOf(spelling));
      });
      return true;
    }

    didBlurWindow() {
      this.activeSelection_ = null;
      this.caretIsHot_ = false;
    }

    /**
     * @param {number} hotStart
     */
    didChangeTextDocument(hotStart) {
      this.updateOffset(hotStart, this.document.length);
      this.caretIsHot_ = true;
      taskScheduler.schedule(this, kHotScanStartDelay);
    }

    didFocusWindow() {
      if (Window.focus instanceof TextWindow) {
        this.activeSelection_ =
            /** @type {!TextSelection} */(Window.focus.selection);
        return;
      }
      this.activeSelection_ = null;
      this.caretIsHot_ = false;
    }

    didLoadTextDocument() {
      this.resetOffset(this.document.length, this.document.length);
      this.caretIsHot_ = false;
    }

    run() {
      this.ensureOffsets();
      this.resetLife(kMaxHotScanCount);
      if (!this.moveToStartOfWord())
        return;
      if (this.caretIsHot_)
        return this.runWithHotCaret();
      this.runWithColdCaret();
    }

    /** @private */
    runWithColdCaret() {
      /** @type {number} */
      let lastOffset = this.offset;
      for (let wordRange of this.words()) {
        this.removeMarker(lastOffset);
        lastOffset = wordRange.end;
        if (!this.checkWord(wordRange.start, wordRange.end)) {
          this.offset_ = wordRange.start;
          lastOffset = wordRange.start;
          break;
        }
      }
      this.removeMarker(lastOffset);
      this.schedule(0);
    }

    /** @private */
    runWithHotCaret() {
      /** @type {number} */
      const caretOffset = this.activeCaretOffset();
      /** @type {number} */
      let lastOffset = this.offset;
      for (let wordRange of this.words()) {
        this.removeMarker(lastOffset);
        lastOffset = wordRange.end;
        if (caretOffset >= wordRange.start && caretOffset <= wordRange.end) {
          // Since candidate word contains caret, we consider this word is still
          // changing.
          this.offset_ = wordRange.start;
          this.caretIsHot_ = false;
          this.schedule(500);
          return;
        }
        if (!this.checkWord(wordRange.start, wordRange.end)) {
          this.offset_ = wordRange.start;
          lastOffset = wordRange.start;
          break;
        }
      }
      this.removeMarker(lastOffset);
      this.schedule(32);
    }

    /**
     * @private
     * @param {number} delay A time in milliseconds to next scan.
     */
    schedule(delay) {
      if (this.offset_ >= this.end_)
        return;
      taskScheduler.schedule(this, delay);
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
  class SpellChecker extends text.SimpleMutationObserverBase {
    /**
     * Spell checker for specified document. Each document is associated to
     * separate |SpellChecker| instance.
     *
     * @param {!TextDocument} document
     */
    constructor(document) {
      super(document);

      /** @type {!TextRange} */
      const range = new TextRange(document);

      /** @type {?} TODO(eval1749): We should use |ColdScanner| here. */
      this.coldScanner_ = new ColdScanner(document);

      /** @type {?} TODO(eval1749): We should use |HotScanner| here. */
      this.hotScanner_ = new HotScanner(range);

      document.addEventListener(Event.Names.ATTACH,
                                this.didAttachWindow.bind(this));

      this.didLoadTextDocument();
    }

    // Cleanup resources used by spell checker.
    destroy() {
      taskScheduler.remove(this.coldScanner_);
      taskScheduler.remove(this.hotScanner_);
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
      this.hotScanner_.didFocusWindow();
    }

    /**
     * @private
     * @param {!Event} event
     * Spell checker is stopped when window loses focus.
     */
    didBlurWindow(event) {
      this.hotScanner_.didBlurWindow();
    }

    /**
     * @private
     * Implements text.SimpleMutationObserver.didChangeTextDocument
     * @param {number} offset
     *
     * Resets hot offset to minimal changed offset and kicks word scanner.
     */
    didChangeTextDocument(offset) {
      this.coldScanner_.didChangeTextDocument(offset);
      this.hotScanner_.didChangeTextDocument(offset);
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
     * Implements text.SimpleMutationObserver.didChangeTextDocument
     */
    didLoadTextDocument() {
      this.coldScanner_.didLoadTextDocument();
      this.hotScanner_.didLoadTextDocument();
    }
  }

  // When document is created/destructed, we install/uninstall spell checker
  // to/from document.
  TextDocument.addObserver(function(action, document) {
    /** @param {!TextDocument} document */
    function installSpellChecker(document) {
      // TODO(eval1749): We should have generic way to disable spell checking
      // for document.
      if (document.name === '*javascript*')
        return;
      const spellChecker = new SpellChecker(document);
      document.properties.set(SpellChecker.name, spellChecker);
    }

    /** @param {!TextDocument} document */
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
