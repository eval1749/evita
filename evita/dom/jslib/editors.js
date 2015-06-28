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
  /**
   * @param {!Os.File} file
   * @param {!TextDecoder} decoder
   * @return {!Promise}
   */
  function readAll(file, decoder) {
    var buffer = new Uint8Array(1024 * 16);
    var text = '';
    function readLoop() {
      return file.read(buffer).then(function(num_bytes) {
        if (!num_bytes) {
          file.close();
          return Promise.resolve(text);
        }
        text += decoder.decode(buffer.subarray(0, num_bytes));
        return readLoop();
      }).catch(function(reason) {
        file.close();
        return Promise.reject(reason);
      });
    }
    return readLoop();
  }

  var options = arguments.length == 1 ? {} :
      /** @type {!Object} */(opt_options);
  var encoding = options.encoding ? /** @type {string} */ (options.encoding) :
      'utf-8';
  var decoder = new TextDecoder(encoding, {fatal: true});
  return Os.File.open(scriptPath).then(function(file) {
    if (options.verbose)
      console.log('/\x2F loading', scriptPath);
    return readAll(file, decoder).then(function(scriptText) {
      var result = Editor.runScript(scriptText, scriptPath);
      if (result.exception && options.verbose) {
        console.log('/\x2A\n' + result.exception + '\n' + result.stackTrace +
            '*/');
      }
      if (options.verbose)
        console.log('/\x2F loaded', scriptPath);
      return result.exception ? Promise.reject(result) : Promise.resolve({});
    }).catch(function(reason) {
      if (options.verbose)
        console.log('/\x2F failed', scriptPath, reason);
      return Promise.reject(reason);
    });
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

  function start(args) {
    loadUserScript();
    var doc = new Document('*scratch*');
    var range = new Range(doc);
    var editor_window = new EditorWindow();
    var text_window = new TextWindow(range);
    editor_window.appendChild(text_window);
    editor_window.realize();
  }
  return start;
})();
