// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('css');

goog.scope(function() {

/** @const @type {string} */
const cssText = [
  // See CSS Basic User Interface Module Level 4
  // caret-shape: auto | bar | block | underscore
  '* { color: #000; background-color: #FFF;',
  '  font-family: Consolas, Meiryo;  font-size: 10;', '  caret-color: #000;',
  '}',

  // Marker symbol
  'control { color: #009900; }', 'marker { color: #009900; }',
  'missing { color: #009900; }',

  // Ruler
  // rgba(0, 0, 0, 0.3)
  '::ruler { color: #0033004D; width: 1; }',

  // Selection
  // See CSS Basic User Interface Module Level 4
  //    rgba(51, 153, 255, 0.3)
  '::selection:active { background-color: #3399FF4D; }',
  //    rgba(191, 205, 219, 0.3)
  '::selection:inactive { background-color: #BFCDDB4D; }',

  // Bracket matching: rgba(238, 255, 65)
  '.bracket { background-color: #EEFF41; }',

  //
  // IME See "evita/dom/windows/text_window.js" for usage of IME classes.
  // See CSS Text Decoration Module
  '.ime_active1 { background-color: #3399FF; color: #FFF; }',
  '.ime_active2 { text-decoration: underline double; }',
  '.ime_inactive1 { text-decoration: underline solid; }',
  '.ime_inactive2 { text-decoration: underline solid #0F0;}',
  '.ime_input { text-decoration: underline wavy; }',
  '.ime_input_error { text-decoration: underline wavy #F00; }',

  //
  // Spelling
  // See CSS Pseudo-Elements Module Level 4
  //    ::selection
  //    ::spelling-error
  //    ::grammar-error
  '.misspelled {text-decoration: underline wavy #F00;}',
  '.bad_grammar {text-decoration: underline wavy #0F0;}',

  ////////////////////////////////////////////////////////////
  //
  // Syntax
  //
  'comment { color: #008000; }', 'html_attribute_name { color: #F00; }',
  'html_attribute_value { color: #00F; }', 'html_comment { color: #008000; }',
  'html_element_name { color: #800000; }', 'html_entity { color: #F00; }',
  'identifier { color: #000000; }', 'keyword { color: #00F; }',
  'keyword2 { color: #0000C0; }', 'label { color: #800080; }',
  'literal { color: #000080; }', 'operators { color: #000080; }',
  'string_literal { color: #A31515; }'
].join('\n');

/** @const @type {!css.CSSStyleSheet} */
const styleSheet = css.Parser.parse(cssText);
Object.defineProperty(TextDocument, 'defaultStyleSheet', {value: styleSheet});
TextWindow.setDefaultStyleSheet(styleSheet['handle_']);
});
