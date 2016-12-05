// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('editors');

goog.scope(function() {

const Initializer = core.Initializer;

/**
 * @return {!Promise}
 */
async function loadUserScript() {
  /** @type {!Array<string>} */
  const dirNames = [Os.getenv('HOME'), Os.getenv('USERPROFILE')];
  for (const dirName of dirNames) {
    if (dirName === '')
      continue;
    const fileName = FilePath.join(dirName, 'evitarc.js');
    // TODO(eval1749): We should not use |repl.load()| or move |repl.load()|
    // here.
    try {
      return await repl.load(fileName, {verbose: true});
    } catch (error) {
      if (error instanceof Os.File.Error)
        return;
      console.log(`Failed to load ${fileName}: ${error}`);
    }
  }
}

/**
 * @param {!Array<string>} args
 */
function processCommandLine(args) {
  /** @const @type {!EditorWindow} */
  const editorWindow = new EditorWindow();
  /** @const @type {!TextDocument} */
  const doc = TextDocument.new('*scratch*');
  /** @const @type {!TextRange} */
  const range = new TextRange(doc);
  /** @const @type {!TextWindow} */
  const textWindow = new TextWindow(range);
  editorWindow.appendChild(textWindow);
  editorWindow.realize();
  args.forEach(fileName => Editor.open(fileName));
}

/**
 * @param {!Array<string>} args
 */
async function start(args) {
  Initializer.initialize();
  Editor.loadModule('commands');
  await loadUserScript()
  processCommandLine(args);
}

/** @const @type {function(!Array<string>)} */
editors.start = start;
});
