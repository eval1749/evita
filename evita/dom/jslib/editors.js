// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.editors = {};

(function() {
  function loadUserScript() {
    var context = {
        dirs: [
            Os.getenv('HOME'),
            Os.getenv('USERPROFILE'),
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
    initialize();
    loadUserScript();
    let editor_window = new EditorWindow();
    let doc = TextDocument.new('*scratch*');
    let range = new TextRange(doc);
    let text_window = new TextWindow(range);
    editor_window.appendChild(text_window);
    editor_window.realize();
    args.forEach(fileName => Editor.open(fileName));
  }

  Object.defineProperties(editors, {
    start: {value: start}
  });
})();
