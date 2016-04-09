// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  Editor.RegExp = RegularExpression;

  Object.defineProperty(Editor, 'activeWindow', {
    /**
     * @return {?Window}
     */
    value: function() {
      var active = null;
      function mostRecentlyUsed(a, b) {
        if (a && b)
          return a.focusTick_ > b.focusTick_ ? a : b;
        return a || b;
      }
      function updateActiveWindow(previous, current) {
        var newValue = mostRecentlyUsed(previous, current);
        return current.children.reduce(updateActiveWindow, newValue);
      }
      return EditorWindow.list.reduce(updateActiveWindow, null);
    }
  });

  /**
   * @param {Function} window_class
   * @param {string} key_combination
   * @param {function(number=)} command
   * @param {string=} opt_description
   */
  Editor.bindKey = function(
      window_class, key_combination, command, opt_description) {
    var key_code = Editor.parseKeyCombination(key_combination);
    if (arguments.length >= 4)
      command['commandDescription'] = opt_description;
    if (!Editor.key_binding_map_map_)
      Editor.key_binding_map_map_ = new Map();
    var map = Editor.key_binding_map_map_.get(window_class);
    if (!map) {
      map = new Map();
      window_class['keymap'] = map;
      Editor.key_binding_map_map_.set(window_class, map);
    }
    map.set(key_code, command);
  };

  Editor.exit = function() {
    var active_window = Editor.activeWindow();

    /**
     * @param {TextDocument} document
     * @return {Promise}
     */
    function interactiveSave(document) {
      return Editor.getFileNameForSave(null, document.fileName)
          .then(function(fileName) {
            if (!fileName.length)
              return Promise.resolve(DialogItemId.CANCEL);
            document.save(fileName);
            return Promise.resolve(DialogItemId.YES);
          });
    }

    /**
     * @param {TextDocument} document
     * @return {Promise}
     */
    function confirmForExit(document) {
      if (!document.needSave())
        return Promise.resolve(true);
      return Editor
          .messageBox(
              null,
              Editor.localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
              MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
          .then(function(code) {
            switch (code) {
              case DialogItemId.CANCEL:
              case DialogItemId.NO:
                return Promise.resolve(code);
              case DialogItemId.YES:
                return interactiveSave(document);
            }
          });
    }

    function isCanceled(answer) { return answer === DialogItemId.CANCEL; }

    Promise.all(TextDocument.list.map(confirmForExit)).then(function(answers) {
      if (answers.some(isCanceled))
        return;
      Editor.forceExit();
    });
  };

  Editor.forceExit = function() {
    EditorWindow.list.forEach(function(window) { window.destroy(); });
  };

  /**
   * @param {string} format_text
   * @param {Object.<string, string>=} opt_dict
   * @return {string}
   * @suppress {globalThis}
   */
  Editor.localizeText = function(format_text, opt_dict) {
    if (arguments.length == 1)
      return format_text;
    var dict = /** @type {!Object} */ (opt_dict);
    var text = format_text;
    Object.keys(dict).forEach(function(key) {
      text = text.replace('__' + key + '__', opt_dict[key]);
    });
    return text;
  };

  /**
   * @param {string} file_name
   */
  Editor.open = function(file_name) {
    const document = TextDocument.open(file_name);
    if (document.length == 0) {
      document.load(file_name).then(function() {
        global.windows.activate(Editor.activeWindow(), document);
      });
      return;
    }
    const parent = Editor.activeWindow().parent;
    const present = parent.children.find((window) => {
      return window instanceof TextWindow && window.document == document;
    });
    if (present) {
      present.focus();
      return;
    }
    const windows = document.listWindows();
    if (windows.length != 0) {
      windows[0].focus();
      return;
    }
    global.windows.newTextWindow(parent, document);
  };

  /**
   * @param {string} spec
   * @return {number} key code.
   * @suppress {globalThis}
   */
  Editor.parseKeyCombination = (function() {
    /** @const */ var MOD_CTRL = 0x200;
    /** @const */ var MOD_SHIFT = 0x400;

    var KEY_COMBINATION_RULES = [
      {modifiers: MOD_CTRL | MOD_SHIFT, re: /^Ctrl[-+]Shift[-+](.+)$/i},
      {modifiers: MOD_CTRL | MOD_SHIFT, re: /^Shift[-+]Ctrl[-+](.+)$/i},
      {modifiers: MOD_CTRL, re: /^Ctrl[-+](.+)$/i},
      {modifiers: MOD_SHIFT, re: /^Shift[-+](.+)$/i},
      {modifiers: 0, re: /^(.+)$/}
    ];

    /**
     * @param {string} spec
     * @return {number} key code.
     */
    function combinationFunction(spec) {
      if (spec.length == 1)
        return spec.charCodeAt(0);
      var code = 0;
      KEY_COMBINATION_RULES.forEach(function(rule) {
        if (code)
          return;
        var matches = rule.re.exec(spec);
        if (!matches)
          return;
        var key_name = matches[1];
        var key_code = KEY_CODE_MAP.get(key_name.toLowerCase());
        if (!key_code)
          throw new Error('Invalid key name: ' + key_name);
        code = rule.modifiers | key_code;
      });
      if (!code)
        throw new Error('Invalid key specification: ' + spec);
      return code;
    }
    return combinationFunction;
  })();

  /**
   * @param {string} cwd
   * @param {!Array.<string>} args
   */
  function processCommandLine(cwd, args) {
    let maybe_option = true;
    for (let arg of args) {
      if (maybe_option && arg[0] === '-') {
        if (arg === '--') {
          maybe_option = false;
          continue;
        }
        // TODO(eval1749): Handle command line option.
        continue;
      }
      if (arg[0] === '*') {
        let document = TextDocument.find(arg);
        if (!document)
          continue;
        windows.activate(Editor.activeWindow(), document);
        continue;
      }
      let path_info = FilePath.split(arg);
      if (path_info.absolute && path_info.winDrive) {
        Editor.open(arg);
        continue;
      }
      Editor.open(cwd + '/' + arg);
    }
  }

  Object.defineProperties(
      Editor, {processCommandLine: {value: processCommandLine}});
})();
