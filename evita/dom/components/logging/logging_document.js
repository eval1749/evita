// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('spell_checker');

goog.scope(function() {

/**
 * @param {string} name
 * @return {!TextDocument}
 */
function ensureDocument(name) {
  const present = TextDocument.find(name);
  if (present)
    return present;
  const document = TextDocument.new(name);
  spell_checker.SpellChecker.disable(document);

  // Bind "Ctrl+L" to clear document.
  document.bindKey(
      'Ctrl+L', function() { document.replace(0, document.length, ''); });

  return document;
}

/** @type {TextRange} */
let loggingRange = null;

/** @return {!TextRange} */
function ensureLoggingRange() {
  if (loggingRange)
    return loggingRange;
  const document = ensureDocument('*logging*');
  loggingRange = new TextRange(document);
  return loggingRange;
}

/**
 * @param {*} thing
 * @return {string}
 */
function safeToString(thing) {
  if (thing === null)
    return 'null';
  if (thing === undefined)
    return 'undefined';
  return thing.toString();
}

// Install DVLOG output handler.
global.DVLOG.output = function(location, args) {
  // TODO(eval1749): We should use |TextDocument.prototype.splice()|.
  const range = ensureLoggingRange();
  const document = range.document;
  range.collapseTo(document.length);
  range.text = `${location.fileName}(${location.lineNumber}):` +
      ` ${location.functionName}: `;
  range.collapseTo(document.length);
  range.text = args.map(arg => safeToString(arg)).join(' ');
  range.collapseTo(document.length);
  range.text = '\n';
  document.clearUndo();
};

});
