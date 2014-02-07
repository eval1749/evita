// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @return {?Window}
   */
  Editor.activeWindow = function() {
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
  };

  /**
   * @param {Function} window_class
   * @param {string} key_combination
   * @param {function(number=)} command
   * @param {string=} opt_description
   */
  Editor.bindKey = function(window_class, key_combination, command,
                            opt_description) {
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
     * @param {Document} document
     * @return {Promise}
     */
    function interactiveSave(document) {
      return Editor.getFilenameForSave(null, document.filename).then(
        function(filename) {
          if (!filename.length)
            return Promise.cast(DialogItemId.CANCEL);
          document.save(filename);
          return Promise.cast(DialogItemId.YES);
        });
    }

    /**
     * @param {Document} document
     * @return {Promise}
     */
    function confirmForExit(document) {
      if (!document.needSave())
        return Promise.cast(true);
      return Editor.messageBox(null,
        Editor.localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
        MessageBox.ICONWARNING | MessageBox.YESNOCANCEL).then(function(code) {
          switch (code) {
            case DialogItemId.CANCEL:
            case DialogItemId.NO:
              return Promise.cast(code);
            case DialogItemId.YES:
              return interactiveSave(document);
           }
        });
    }

    /** @param {DialogItemId} answer */
    function isCanceled(answer) {
      return answer == DialogItemId.CANCEL;
    }

    Promise.all(Document.list.map(confirmForExit)).then(function(answers) {
      if (answers.some(isCanceled))
        return;
      Editor.forceExit();
    });
  };

  Editor.forceExit = function() {
    EditorWindow.list.forEach(function(window) {
      window.destroy();
    });
  };

  /**
   * @param {?Window} window.
   * @param {string} dirname.
   * @return {Promise}
   */
  Editor.getFilenameForLoad = function(window, dirname) {
    var deferred = Promise.defer();
    Editor.getFilenameForLoad_(window || Editor.activeWindow(), dirname,
                               deferred.resolve);
    return deferred.promise;
  };

  /**
   * @param {?Window} window.
   * @param {string} dirname.
   * @return {Promise}
   */
  Editor.getFilenameForSave = function(window, dirname) {
    var deferred = Promise.defer();
    Editor.getFilenameForSave_(window || Editor.activeWindow(), dirname,
                               deferred.resolve);
    return deferred.promise;
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
    var dict = /** @type {!Object} */(opt_dict);
    var text = format_text;
    Object.keys(dict).forEach(function(key) {
      text = text.replace('__' + key + '__', opt_dict[key]);
    });
    return text;
  };

  /**
   * @param {?Window} window.
   * @param {string} message
   * @param {number} flags
   * @param {string=} opt_title
   * @return {!Promise.<number>}
   */
  Editor.messageBox = function (window, message, flags, opt_title) {
    var title = arguments.length >= 4 ? /** @type{string} */(opt_title) : '';
    var deferred = Promise.defer();
    Editor.messageBox_(window || Editor.activeWindow(), message, flags, title,
                       deferred.resolve);
    return /** @type{!Promise.<number>} */(deferred.promise);
  };

  /**
   * @param {string} spec.
   * @return {number} key code.
   * @suppress {globalThis}
   */
  Editor.parseKeyCombination = (function() {
    /** @const */ var MOD_CTRL = 0x200;
    /** @const */ var MOD_SHIFT = 0x400;

    var KEY_COMBINATION_RULES = [
      {
        modifiers: MOD_CTRL | MOD_SHIFT,
        re: /^Ctrl[-+]Shift[-+](.+)$/i
      },
      {
        modifiers: MOD_CTRL | MOD_SHIFT,
        re: /^Shift[-+]Ctrl[-+](.+)$/i
      },
      {
        modifiers: MOD_CTRL,
        re: /^Ctrl[-+](.+)$/i
      },
      {
        modifiers: MOD_SHIFT,
        re: /^Shift[-+](.+)$/i
      },
      {
        modifiers: 0,
        re: /^(.+)$/
      }
    ];

    /**
     * @param {string} spec.
     * @return {number} key code.
     */
    return function(spec) {
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
        var key_code = KEY_CODE_MAP[key_name.toLowerCase()];
        if (!key_code)
          throw new Error('Invalid key name: ' + key_name);
        code = rule.modifiers | key_code;
      });
      if (!code)
        throw new Error('Invalid key specification: ' + spec);
      return code;
    }
  })();
})();
