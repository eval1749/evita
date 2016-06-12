// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('commanders');
goog.require('commands');
goog.require('testing');

goog.scope(() => {

/** @constructor */
const Command = commanders.Command;

/**
 * @param {string} text
 * @param {number} start
 * @param {number} end
 * @return {!TextWindow}
 */
function setup(text, start, end) {
  if (testing.gmock)
    testing.gmock.expectCallCreateTextWindow(1);
  const document = new TextDocument();
  const window = new TextWindow(new TextRange(document));
  document.replace(0, 0, text);
  window.selection.range.start = start;
  window.selection.range.end = end;
  return window;
}

testing.test('TextWindow.toCapitalCase.collapsed', (t) => {
  const window = setup('foo bar baz', 4, 4);
  const document = window.document;
  Command.query('TextWindow.toCapitalCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('foo Bar baz');
});

testing.test('TextWindow.toCapitalCase.range', (t) => {
  const window = setup('foo bar BAZ', 2, 6);
  const document = window.document;
  Command.query('TextWindow.toCapitalCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('foO bar BAZ');
});

testing.test('TextWindow.toLowerCase.collapsed', (t) => {
  const window = setup('FOO BAR BAZ', 4, 4);
  const document = window.document;
  Command.query('TextWindow.toLowerCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('FOO bar BAZ');
});

testing.test('TextWindow.toLowerCase.range', (t) => {
  const window = setup('FOO BAR BAZ', 2, 6);
  const document = window.document;
  Command.query('TextWindow.toLowerCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('FOo baR BAZ');
});

testing.test('TextWindow.toUpperCase.collapsed', (t) => {
  const window = setup('foo bar baz', 4, 4);
  const document = window.document;
  Command.query('TextWindow.toUpperCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('foo BAR baz');
});

testing.test('TextWindow.toUpperCase.range', (t) => {
  const window = setup('foo bar BAZ', 2, 6);
  const document = window.document;
  Command.query('TextWindow.toUpperCase').execute(window);
  t.expect(document.slice(0, document.length)).toEqual('foO BAr BAZ');
});

});
