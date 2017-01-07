// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

Editor['RegExp'] = RegularExpression;

/**
 * @param {?Window} window1
 * @param {!Window} window2
 * @return {?Window}
 */
function mostRecentlyUsed(window1, window2) {
  if (window1 && window2)
    return window1.focusTick_ > window2.focusTick_ ? window1 : window2;
  return window1 || window2;
}


/**
 * @return {?Window}
 */
function activeWindow() {
  /**
   * @param {?Window} previous
   * @param {!Window} current
   * @return {?Window}
   */
  function updateActiveWindow(previous, current) {
    const newValue = mostRecentlyUsed(previous, current);
    return current.children.reduce(updateActiveWindow, newValue);
  }
  return EditorWindow.list.reduce(updateActiveWindow, null);
}

function exit() {
  /**
   * @param {!TextDocument} document
   * @return {!Promise}
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
   * @param {!TextDocument} document
   * @return {!Promise<DialogItemId>}
   */
  function confirmForExit(document) {
    if (!document.needSave())
      return Promise.resolve(DialogItemId.YES);
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

  /**
   * TODO(eval1749): We should use |DialogItemId| instead of |DialogItemId| or
   * |Promise<DialogItemId>|, once closure compiler fixes it. V20160310
   * @param {DialogItemId|Promise<DialogItemId>} answer
   * @return {boolean}
   */
  function isCanceled(answer) { return answer === DialogItemId.CANCEL; }

  Promise.all(TextDocument.list.map(confirmForExit)).then(answers => {
    if (answers.some(isCanceled))
      return;
    Editor.forceExit();
  });
}

function forceExit() {
  EditorWindow.list.forEach(window => window.destroy());
}

/**
 * @param {string} format_text
 * @param {Object<string, string>=} opt_dict
 * @return {string}
 * @suppress {globalThis}
 */
function localizeText(format_text, opt_dict) {
  if (arguments.length === 1)
    return format_text;
  var dict = /** @type {!Object} */ (opt_dict);
  var text = format_text;
  Object.keys(dict).forEach(function(key) {
    text = text.replace('__' + key + '__', opt_dict[key]);
  });
  return text;
};

/**
 * @param {string} fileName
 */
function open(fileName) {
  const document = TextDocument.open(fileName);
  if (document.length === 0) {
    document.load(fileName).then(function() {
      global.windows.activate(Editor.activeWindow(), document);
    });
    return;
  }
  const parent = Editor.activeWindow().parent;
  const present = parent.children.find((window) => {
    return window instanceof TextWindow && window.document === document;
  });
  if (present) {
    present.focus();
    return;
  }
  const windows = document.listWindows();
  if (windows.length !== 0) {
    windows[0].focus();
    return;
  }
  global.windows.newTextWindow(parent, document);
};

/** @const @type {number} */ const MOD_CTRL = 0x200;
/** @const @type {number} */ const MOD_SHIFT = 0x400;
/** @const @type {!Array<{modifiers: number, re: !RegExp}>} */
const KEY_COMBINATION_RULES = [
  {modifiers: MOD_CTRL | MOD_SHIFT, re: /^Ctrl[-+]Shift[-+](.+)$/i},
  {modifiers: MOD_CTRL | MOD_SHIFT, re: /^Shift[-+]Ctrl[-+](.+)$/i},
  {modifiers: MOD_CTRL, re: /^Ctrl[-+](.+)$/i},
  {modifiers: MOD_SHIFT, re: /^Shift[-+](.+)$/i}, {modifiers: 0, re: /^(.+)$/}
];

/**
 * @param {string} spec
 * @return {number} key code.
 */
function parseKeyCombination(spec) {
  if (spec.length === 1)
    return spec.charCodeAt(0);
  let code = 0;
  KEY_COMBINATION_RULES.forEach(function(rule) {
    if (code)
      return;
    var matches = rule.re.exec(spec);
    if (!matches)
      return;
    var keyName = matches[1];
    var keyCode = KEY_CODE_MAP.get(keyName.toLowerCase());
    if (!keyCode)
      throw new Error('Invalid key name: "${keyName}"');
    code = rule.modifiers | keyCode;
  });
  if (!code)
    throw new Error('Invalid key specification: "${spec}"');
  return code;
}

/**
 * @param {string} cwd
 * @param {!Array<string>} args
 */
function processCommandLine(cwd, args) {
  /** @type {boolean} */
  let maybeOption = true;
  for (let arg of args) {
    if (maybeOption && arg[0] === '-') {
      if (arg === '--') {
        maybeOption = false;
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
    /** @const @type {!FilePathSplit} */
    const path_info = FilePath.split(arg);
    if (path_info.absolute && path_info.winDrive) {
      Editor.open(arg);
      continue;
    }
    Editor.open(cwd + '/' + arg);
  }
}

[activeWindow, exit, forceExit, localizeText, open, parseKeyCombination,
 processCommandLine,
].forEach(fn => Object.defineProperty(Editor, fn.name, {value: fn}));

});
