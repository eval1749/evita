// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('suggestions');
goog.require('testing');

goog.scope(() => {

class Sample {
  /**
   * @param {string} sampleText
   * @param {!Array<string>} words
   */
  constructor(sampleText, words) {
    this.document_ = new TextDocument();
    const prefixEnd = sampleText.indexOf('|');
    this.document_.replace(0, 0, sampleText.replace('|', ''));
    this.range_ = new TextRange(this.document_, prefixEnd);
    this.session_ = new suggestions.Session(this.range_);
    this.session_.verbose = 2;
    this.session_.start();
  }

  /**
   * @public
   * @return {!Array<string>}
   */
  advance() {
    /** @const @type {string} */
    const response = this.session_.advance();
    /** @const @type {string} */
    const suggestion =
        this.document_.slice(this.session_.anchor, this.range_.end);
    return [suggestion, response];
  }
}

testing.test('Suggestions.basic', (t) => {
  const sample = new Sample('Fun foo bar foo baz f| fox foot', ['funny']);

  t.expect(sample.advance()).toEqual(['foo', 'Suggestion is "foo".']);
  t.expect(sample.advance(), 'No duplicated suggestion').toEqual([
    'Fun', 'Suggestion is "Fun".'
  ]);
  t.expect(sample.advance()).toEqual(['fox', 'Suggestion is "fox".']);
  t.expect(sample.advance()).toEqual(['foot', 'Suggestion is "foot".']);
  t.expect(sample.advance()).toEqual(['foot', 'No more words start with "f"']);
});

testing.test('Suggestions.noSuggestion', (t) => {
  const sample = new Sample('foo bar foo baz x| fox foot', []);
  t.expect(sample.advance()).toEqual(['x', 'No word starts with "x"']);
});

testing.test('Suggestions.OneSuggestion', (t) => {
  const sample = new Sample('foo bar foo ba| fox foot', []);
  t.expect(sample.advance()).toEqual(['bar', 'Suggestion is "bar".']);
  t.expect(sample.advance()).toEqual(['bar', 'No more words start with "ba"']);
});

});
