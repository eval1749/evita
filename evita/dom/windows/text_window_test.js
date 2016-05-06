// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require("testing");
goog.require("windows");

goog.scope(function() {

/**
 * @param {!TextWindow} window
 * @return {string}
 */
function markersOf(window) {
  /** @const @type {!Array<string>} */
  const markers = new Array();
  for (let offset = 0; offset < window.document.length; ++offset)
    markers.push((window.markerAt(offset) + '.').substr(0, 1));
  return markers.join('');
}

testing.test('windows.highlightMatchedBrackets', function(t) {
  testing.gmock.expectCallCreateTextWindow(1);
  const sample = new TextWindow(new TextRange(new TextDocument()));
  sample.document.replace(0, 0, ' (foo)');

  console.log('Bracket.DATA', Bracket.DATA[0x28], Bracket.DATA[0x29]);

  // Move after right bracket
  sample.selection.range.collapseTo(6);
  windows.highlightMatchedBrackets(sample);
  t.expect(markersOf(sample)).toEqual('.b...b');

  // insert 'x' after right bracket
  sample.document.replace(6, 6, 'x');
  windows.highlightMatchedBrackets(sample);
  t.expect(markersOf(sample)).toEqual('.......');

  // Move after right bracket
  sample.selection.range.collapseTo(6);
  windows.highlightMatchedBrackets(sample);
  t.expect(markersOf(sample)).toEqual('.b...b.');
});

testing.test('TextWindow.prototype.markerAt', function(t) {
  testing.gmock.expectCallCreateTextWindow(1);
  const sample = new TextWindow(new TextRange(new TextDocument()));
  sample.document.replace(0, 0, '0123456789');
  sample.setMarker(3, 5, 'foo');

  t.expect(sample.markerAt(0)).toEqual('');
  t.expect(sample.markerAt(3)).toEqual('foo');
  t.expect(sample.markerAt(4)).toEqual('foo');
  t.expect(sample.markerAt(5)).toEqual('');
});

testing.test('TextWindow.prototype.zoom', function(t) {
  testing.gmock.expectCallCreateTextWindow(1);
  const sample = new TextWindow(new TextRange(new TextDocument()));
  sample.zoom = 1.5;

  t.expect(sample.zoom).toEqual(1.5);

  try {
    sample.zoom = 0;
  } catch (error) {
    t.expect(error.toString())
        .toEqual(
            "RangeError: Failed to set the 'zoom' property on 'TextWindow': " +
            "TextWindow zoom must be greater than zero.");
  }

  try {
    sample.zoom = -1;
  } catch (error) {
    t.expect(error.toString())
        .toEqual(
            "RangeError: Failed to set the 'zoom' property on 'TextWindow': " +
            "TextWindow zoom must be greater than zero.");
  }
});

});
