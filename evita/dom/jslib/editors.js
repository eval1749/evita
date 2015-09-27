// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

global.editors = {};

/**
 * @param {string} scriptPath
 * @param {!Object=} opt_options
 * @return {!Promise}
 *
 * Options:
 *  encoding: string
 *  verbose: boolean
 */
function load(scriptPath, opt_options) {
  let options = opt_options ? /** @type{!Object} */(opt_options) : {};
  let encoding = options.encoding ? /** @type{string} */(options.encoding)
      : 'utf-8';
  if (options.verbose)
    console.log('/\x2F loading', scriptPath);
  /** @type {Os.File} */
  let file = null;
  return (async(function*() {
    file = yield Os.File.open(scriptPath);
    /** @type {number} */
    let total_read = 0;
    /** @type {string} */
    let text = '';
    /** @type {!ArrayBufferView} */
    const buffer = new Uint8Array(4096);
    /** @type {!TextDecoder} */
    const decoder = new TextDecoder(encoding, {fatal: true});
    for (;;) {
      let num_read = yield file.read(buffer);
      if (num_read == 0)
        break;
      text += decoder.decode(buffer.subarray(0, num_read));
      total_read += num_read;
    }
    file.close();
    if (options.verbose)
      console.log('/\x2F loaded', scriptPath, total_read);
    let result = Editor.runScript(text, scriptPath);
    if (result.exception && options.verbose) {
      console.log('/\x2A\n' + result.exception + '\n' + result.stackTrace +
                  '*/');
    }
    if (result.exception)
      return Promise.reject(result.exception)
    return Promise.resolve();
  }))().catch((reason) => {
    if (file)
      file.close();
    console.log('/\x2F failed', scriptPath, reason);
    throw reason;
  });
}

/** @type {function(!Array.<string>)} */
editors.start = (function() {
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
      return load(context.scriptPath, {verbose: true});
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
    loadUserScript();
    let editor_window = new EditorWindow();
    let doc = new Document('*scratch*');
    let range = new Range(doc);
    let text_window = new TextWindow(range);
    editor_window.appendChild(text_window);
    editor_window.realize();
    args.forEach(fileName => Editor.open(fileName));
  }
  return start;
})();
