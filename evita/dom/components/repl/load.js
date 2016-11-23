// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
const LINE_COMMENT = '\x2F/';

/**
 * @param {string} scriptPath
 * @param {!Object=} options
 * @return {!Promise}
 *
 * Options:
 *  encoding: string
 *  verbose: boolean
 */
async function load(scriptPath, options = {}) {
  /** @const @type {string} */
  const encoding =
      options.encoding ? /** @type{string} */ (options.encoding) : 'utf-8';
  if (options.verbose)
    console.log(LINE_COMMENT, 'loading', scriptPath);
  /** @type {Os.File} */
  let file = await Os.File.open(scriptPath);
  try {
    /** @type {number} */
    let totalRead = 0;
    /** @type {string} */
    let text = '';
    /** @type {!ArrayBufferView} */
    const buffer = new Uint8Array(4096);
    /** @type {!TextDecoder} */
    const decoder = new TextDecoder(encoding, {fatal: true});
    for (;;) {
      /** @const @type {number} */
      const numRead = await file.read(buffer);
      if (numRead === 0)
        break;
      text += decoder.decode(buffer.subarray(0, numRead));
      totalRead += numRead;
    }
    file.close();
    file = null;
    text = text.replace(/\r\n/g, '\n');
    if (options.verbose)
      console.log(LINE_COMMENT, 'loaded', scriptPath, totalRead);
    /** @const @type {!RunScriptResult} */
    const result = Editor.runScript(text, scriptPath);
    if (result.exception instanceof SyntaxError) {
      const syntaxError = result.exception + '; ' + scriptPath + '(' +
          result.lineNumber + ':' + result.startColumn + ':' +
          result.endColumn + ')';
      throw syntaxError;
    }
    if (result.exception)
      throw result.exception;
    return totalRead;
  } finally {
    if (file)
      file.close();
  }
}

/**
 * @param {string} scriptPath
 * @param {!Object=} opt_options
 * @return {!Promise}
 */
repl.load = load;
});

global.load = repl.load;
