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
    var window = Editor.activeWindow();
    var can_exit = true;
    Document.list.forEach(function(document) {
      if (!document.needSave())
        return;
      can_exit = false;
      // TODO(yosi) How do we handle [Cancel] button?
      document.close();
    });
    if (!can_exit)
      return;
    Editor.forceExit();
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
    var resolved;
    var promise = new Promise(function(r) { resolved = r; });
    Editor.getFilenameForLoad_(window || Editor.activeWindow(), dirname,
                               resolved);
    return promise;
  };

  /**
   * @param {?Window} window.
   * @param {string} dirname.
   * @return {Promise}
   */
  Editor.getFilenameForSave = function(window, dirname) {
    var resolved;
    var promise = new Promise(function(r) { resolved = r; });
    Editor.getFilenameForSave_(window || Editor.activeWindow(), dirname,
                               resolved);
    return promise;
  };

  /**
   * @param {?Window} window.
   * @param {string} message
   * @param {string} title
   * @param {number} flags
   * @return {Promise}
   */
  Editor.messageBox = function(window, message, title, flags) {
    var resolved;
    var promise = new Promise(function(r) { resolved = r; });
    Editor.messageBox_(window || Editor.activeWindow(), message, title, flags,
                       resolved);
    return promise;
  };

  /**
   * @param {string} combination.
   * @param {function} command.
   */
  Editor.setKeyBinding = function(combination, command) {
    Editor.setKeyBinding_(parseKeyCombination(combination), command);
  };
})();
