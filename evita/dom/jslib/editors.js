// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('editors');

goog.scope(function() {

const Initializer = core.Initializer;

function loadUserScript() {
  var context = {
    dirs: [
      Os.getenv('HOME'), Os.getenv('USERPROFILE'),
      Os.getenv('HOMEDRIVE') + Os.getenv('HOMEPATH')
    ],
    scriptPath: ''
  };
  function tryLoad() {
    if (!context.dirs.length)
      return Promise.reject('none');
    context.scriptPath = FilePath.join(context.dirs.shift(), 'evitarc.js');
    return repl.load(context.scriptPath, {verbose: true});
  }
  return tryLoad().catch(function(reason) {
    if (!(reason instanceof Os.File.Error))
      return;
    tryLoad();
  });
}

/**
 * @param {!Array.<string>} args
 */
function start(args) {
  Initializer.initialize();
  loadUserScript();
  /** @const @type {!EditorWindow} */
  const editorWindow = new EditorWindow();
  /** @const @type {!TextDocument} */
  const doc = TextDocument.new('*scratch*');
  doc.mode = Mode.create('plain');
  /** @const @type {!TextRange} */
  const range = new TextRange(doc);
  /** @const @type {!TextWindow} */
  const textWindow = new TextWindow(range);
  editorWindow.appendChild(textWindow);
  editorWindow.realize();
  args.forEach(fileName => Editor.open(fileName));
}

/** @const @type {function(!Array.<string>)} */
editors.start = start;
});
