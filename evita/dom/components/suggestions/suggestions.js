// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('suggestions');

goog.require('base.Logger');

goog.scope(function() {

const Logger = base.Logger;

/** @interface */
const Provider = function() {};

/** @return {boolean} */
Provider.prototype.isReady = function() {};

/** @return {string} */
Provider.prototype.next = function() {};

/**
 * @typedef {function(new:Provider, !Session)}
 */
var ProviderFactory

    /** @const @type {!Map<!TextDocument, !Session>} */
    const staticSessionMap = new Map();

/** @const @type {!Array<!ProviderFactory>} */
const staticProvider = [];

/** @type {number} */
let staticVerbose = 0;

/**
 * @param {string} text
 * @param {!TextDocument} document
 * @param {number} start
 * @return {boolean}
 */
function matchAt(text, document, start) {
  /** @type {number} */
  let offset = start;
  for (/** @type {number} */ let k = 0; k < text.length; ++k) {
    /** @const @type {number} */
    const code1 = base.toAsciiLowerCase(document.charCodeAt(offset));
    /** @const @type {number} */
    const code2 = base.toAsciiLowerCase(text.charCodeAt(k));
    if (code1 !== code2)
      return false;
    ++offset;
  }
  return true;
}

/**
 * @param {string} text
 * @param {!TextRange} range
 * @param {number} direction
 * @return {boolean}
 */
function findWordStartsWith(text, range, direction) {
  console.assert(text !== '');
  console.assert(direction !== 0);
  /** @const @type {!TextDocument} */
  const document = range.document;
  /** @const @type {number} */
  const textLength = text.length;
  if (direction > 0) {
    /** @const @type {number} */
    const maxStart = document.length - textLength;
    do {
      range.move(Unit.WORD, 1);
      if (range.start > maxStart)
        break;
      if (matchAt(text, document, range.start))
        return true;
    } while (range.start < maxStart);
    range.collapseTo(document.length);
    return false;
  }
  if (range.start === 0)
    return false;
  do {
    range.move(Unit.WORD, -1);
    if (range.start + textLength > document.length)
      break;
    if (matchAt(text, document, range.start))
      return true;
  } while (range.start);
  range.collapseTo(0);
  return false;
}

/**
 * @implements {Provider}
 */
class SpellingProvider extends Logger {
  /**
   * @public
   * @param {!Session} session
   */
  constructor(session) {
    super();
    /** @type {boolean} */
    this.isReady_ = session.prefix.length <= 3;
    /** @type {!Array<string>} */
    this.words_ = [];
    if (this.isReady_)
      return;
    Editor.getSpellingSuggestions(session.prefix).then(words => {
      this.isReady_ = true;
      this.words_ = words;
    });
  }

  /**
   * implements Provider.prototype.isReady
   * @return {boolean}
   */
  isReady() { return this.isReady_; }

  /**
   * implements Provider.prototype.next
   * @return {string}
   */
  next() {
    console.assert(this.isReady);
    if (this.words_.length === 0)
      return '';
    /** @const @type {string} */
    const word = this.words_.shift();
    this.log(1, `got '${word}'`);
    return word;
  }
}

/**
 * @implements {Provider}
 * Extracts words from text document starts with specified prefix.
 */
class WordExtracter extends Logger {
  /**
   * @public
   * @param {!Session} session
   */
  constructor(session) {
    super();
    /** @const @type {number} */
    this.anchor_ = session.anchor;
    /** @const @type {!TextDocument} */
    this.document_ = session.document;
    /** @type {number} */
    this.direction_ = -1;
    /** @const @type {string} */
    this.prefix_ = session.prefix;
    /** @const @type {!TextRange} */
    this.range_ = new TextRange(session.document, session.anchor);
  }

  /**
   * @private
   * @return {string}
   */
  extractBackward() {
    /** @const @type {!TextRange} */
    const range = this.range_;
    range.collapseTo(range.start);
    while (range.start > 0) {
      range.move(Unit.WORD, -1);
      if (matchAt(this.prefix_, this.document_, range.start)) {
        range.endOf(Unit.WORD, Alter.EXTEND);
        return range.text;
      }
    }
    return '';
  }

  /**
   * @private
   * @return {string}
   */
  extractForward() {
    /** @const @type {!TextRange} */
    const range = this.range_;
    range.collapseTo(range.end);
    /** @const @type {number} */
    const maxOffset = this.document_.length - this.prefix_.length - 1;
    while (range.start < maxOffset) {
      if (matchAt(this.prefix_, this.document_, range.start)) {
        range.endOf(Unit.WORD, Alter.EXTEND);
        return range.text;
      }
      range.move(Unit.WORD, 1);
    }
    return '';
  }

  /**
   * implements Provider.prototype.isReady
   * @public
   * @return {boolean}
   */
  isReady() { return true; }

  /**
   * implements Provider.prototype.next
   * @public
   * @return {string}
   */
  next() {
    this.log(1, 'next', this.range_);
    switch (this.direction_) {
      case 0:
        return '';
      case 1: {
        /** @type {string} */
        const word = this.extractForward();
        if (word !== '') {
          this.log(1, 'next', `found '${word}'`);
          return word;
        }
        this.direction_ = 0;
        return '';
      }
      case -1: {
        /** @type {string} */
        const word = this.extractBackward();
        if (word !== '') {
          this.log(1, 'next', `found '${word}'`);
          return word;
        }
        this.range_.collapseTo(this.anchor_);
        this.range_.move(Unit.WORD, 1);
        this.direction_ = 1;
        return this.next();
      }
    }
    throw new Error('Invalid direction', this);
  }

  /** @override */
  toString() { return `WordExtracter(${this.direction_})`; }
}

class Session extends Logger {
  /**
   * @public
   * @param {!TextRange} range
   */
  constructor(range) {
    super();
    console.assert(range.collapsed, range);
    range.moveStart(Unit.WORD, -1);
    /** @const @type {number} */
    this.anchor_ = range.start;
    /** @const @type {!TextDocument} */
    this.document_ = range.document;
    /** @type {number} */
    this.index_ = 0;
    /** @type {!Array<!Provider>} */
    this.providers_ = [];
    /** @type {!TextRange} */
    this.range_ = range;
    /** @type {!Array<string>} */
    this.words_ = [range.text];
    range.collapseTo(range.end);
  }

  /** @return {number} */
  get anchor() { return this.anchor_; }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /** @return {string} */
  get lastWord() { return this.words_[this.index_]; }

  /** @return {string} */
  get prefix() { return this.words_[0]; }

  /** @override */
  didChangeVerbose() {
    for (const provider of this.providers_)
      provider.verbose = this.verbose;
  }

  /**
   * @private
   */
  acceptSuggestion() {
    console.assert(this.index_ < this.words_.length, this);
    /** @const @type {string} */
    const suggestion = this.words_[this.index_];
    this.log(0, 'acceptSuggestion', `'${suggestion}'`);
    this.document_.replace(this.anchor_, this.range_.end, suggestion);
  }

  /**
   * @public
   * @return {string}
   */
  advance() {
    if (this.isFinishCollecting()) {
      if (this.words_.length === 1)
        return `No suggestion starts with "${this.prefix}"`;
      this.index_ = (this.index_ + 1) % this.words_.length;
      this.acceptSuggestion();
      return `Suggestion ${this.index_ + 1}/${this.words_.length}`;
    }
    /** @type {string} */
    const response = this.updateSuggetions();
    if (response !== '')
      return response;
    this.index_ = (this.index_ + 1) % this.words_.length;
    this.acceptSuggestion();
    return `Suggestion is "${this.lastWord}".`;
  }

  /**
   * @private
   * @param {!TextRange} range
   * @return {boolean} True if |this| session is active.
   */
  isActive(range) {
    if (this.words_.length === 0) {
      this.log(1, 'No word');
      return false;
    }
    if (this.range_ !== range) {
      this.log(1, 'Someone invokes session in different window.');
      return false;
    }
    if (!this.range_.collapsed) {
      this.log(1, 'Selection is moved.');
      return false;
    }
    if (this.anchor_ !== range.end - this.lastWord.length) {
      this.log(1, 'Selection is moved.');
      return false;
    }
    if (!matchAt(this.lastWord, this.document_, this.anchor_)) {
      this.log(1, 'Selection contains another word.');
      return false;
    }
    return true;
  }

  /**
   * @private
   * @return {boolean}
   */
  isFinishCollecting() { return this.providers_.length === 0; }

  /**
   * @private
   */
  start() {
    console.assert(this.index_ === 0);
    console.assert(this.providers_.length === 0);
    console.assert(this.words_.length === 1);
    if (this.prefix === '')
      return;
    this.providers_ = staticProvider.map(provider => new provider(this));
    this.didChangeVerbose();
  }

  /** @override */
  toString() {
    return `suggestions.Session('${this.document_.name}' '${this.prefix}'` +
        ` range: ${this.anchor}, ${this.range_.end},` +
        ` words: ${this.index_ + 1}/${this.words_.length})`;
  }

  /**
   * @private
   * @return {string}
   */
  updateSuggetions() {
    if (this.isFinishCollecting()) {
      if (this.words_.length > 1)
        return `No more words start with "${this.prefix}"`;
      return `No word starts with "${this.prefix}"`;
    }
    /** @type {!Provider} */
    const provider = this.providers_[0];
    if (!provider.isReady())
      return 'Collecting suggestion ...';
    for (;;) {
      const suggestion = provider.next();
      if (suggestion === '') {
        this.providers_.shift();
        return this.updateSuggetions();
      }
      if (this.words_.includes(suggestion))
        continue;
      this.log(0, 'updateSuggetions', `add '${suggestion}'`);
      this.words_.push(suggestion);
      return '';
    }
  }

  /** @public @return {number} */
  static get verbose() { return staticVerbose; }

  /** @public @param {number} newVerbose */
  static set verbose(newVerbose) { staticVerbose = newVerbose; }

  /**
   * @public
   * @param {!ProviderFactory} factory
   */
  static addProvider(factory) { staticProvider.push(factory); }

  /**
   * @public
   * @param {!TextRange} range
   * @return {string}
   */
  static expand(range) {
    if (!range.collapsed)
      return '';
    /** @const @type {?Session} */
    const session = Session.for (range.document);
    if (session && session.isActive(range))
      return session.advance();
    /** @const @type {!Session} */
    const newSession = new Session(range);
    staticSessionMap.set(range.document, newSession);
    newSession.verbose = Session.verbose;
    newSession.start();
    return newSession.advance();
  }

  /**
   * @private
   * @param {!TextDocument} document
   * @return {?Session}
   */
  static for (document) { return staticSessionMap.get(document) || null; }
}

Session.addProvider(WordExtracter);
Session.addProvider(SpellingProvider);

/** @this {!TextWindow} */
function expandCommand() {
  this.status = Session.expand(this.selection.range);
}

/** @interface */
suggestions.Provider = Provider;

/** @constructor */
suggestions.Session = Session;

Editor.bindKey(TextWindow, 'Ctrl+/', expandCommand);
});
