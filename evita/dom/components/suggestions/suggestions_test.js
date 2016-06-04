// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('suggestions');
goog.require('testing');

goog.scope(() => {

class Sample {
  /**
   * @param {string} sampleText
   * @param {string} prefix
   */
  constructor(sampleText, prefix) {
    this.document_ = new TextDocument();
    this.start_ = sampleText.indexOf('|');
    this.document_.replace(0, 0, sampleText.replace('|', ''));
    this.range_ = new TextRange(this.document_, this.start_);
    this.session_ = suggestions.Session.getOrCreate(this.document_);
  }

  /**
   * @public
   * @return {!Array<string>}
   */
  advance() {
    const range = this.range_;
    const response = this.session_.expand(range);
    if (response === '')
      range.moveStart(Unit.WORD, -1);
    const suggestion = range.text;
    range.collapseTo(range.end);
    return [suggestion, response];
  }
}

testing.test('Suggestions.basic', (t) => {
  const sample = new Sample('fun foo bar foo baz f| fox foot');

  t.expect(sample.advance()).toEqual(['foo', '']);
  t.expect(sample.advance(), 'No duplicated suggestion').toEqual(['fun', '']);
  t.expect(sample.advance()).toEqual(['fox', '']);
  t.expect(sample.advance()).toEqual(['foot', '']);
  t.expect(sample.advance(), 'Start over').toEqual(['foo', '']);
});

testing.test('Suggestions.noSuggestion', (t) => {
  const sample = new Sample('foo bar foo baz x| fox foot');
  t.expect(sample.advance()).toEqual(['x', 'No word starts with "x"']);
});

testing.test('Suggestions.OneSuggestion', (t) => {
  const sample = new Sample('foo bar foo ba| fox foot');
  t.expect(sample.advance()).toEqual(['bar', '']);
  t.expect(sample.advance()).toEqual(['bar', 'No more words start with "ba"']);
});

});
