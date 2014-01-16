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
        localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
        localizeText(Strings.IDS_APP_TITLE),
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
   * @param {?Window} window.
   * @param {string} message
   * @param {string} title
   * @param {number} flags
   * @return {Promise}
   */
  Editor.messageBox = function(window, message, title, flags) {
    var deferred = Promise.defer();
    Editor.messageBox_(window || Editor.activeWindow(), message, title, flags,
                       deferred.resolve);
    return deferred.promise;
  };

  /**
   * @param {string} combination.
   * @param {function(number=)} command.
   */
  Editor.setKeyBinding = function(combination, command) {
    Editor.setKeyBinding_(parseKeyCombination(combination), command);
  };
})();
