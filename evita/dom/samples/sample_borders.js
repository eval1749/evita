// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('samples');

goog.scope(function() {

const CSSStyleSheet = css.CSSStyleSheet;
const Document = visuals.Document;
const Element = visuals.Element;

/** @return {!Document} */
function createDoucment() {
  /** @const @type {!Document} */
  const document = new Document();
  /** @const @type {!Element} */
  const body = document.createElement('body');
  document.appendChild(body);
  for (let width = 1; width < 10; ++width) {
    /** @const @type {!Element} */
    const div = document.createElement('div');
    div.classList.add(`border${width}`);
    div.appendChild(document.createText(`size=${width}`));
    body.appendChild(div);
  }
  return document;
}

/** @return {!CSSStyleSheet} */
function createStyleSheet() {
  /** @const @type {string} */
  const cssText = 'body {\n' +
      '  display: block;\n' +
      '  margin: 0 4 0 4;\n' +
      '}\n' +
      '.border1 { border: 1 solid #00f; }\n' +  // 83×53
      '.border2 { border: 2 solid #00f; }\n' +  // 85×55
      '.border3 { border: 3 solid #00f; }\n' +
      '.border4 { border: 4 solid #00f; }\n' +
      '.border5 { border: 5 solid #00f; }\n' +
      '.border6 { border: 6 solid #00f; }\n' +
      '.border7 { border: 7 solid #00f; }\n' +
      '.border8 { border: 8 solid #00f; }\n' +
      '.border9 { border: 9 solid #00f; }\n' +
      'div { display: inline; margin: 10; width: 80; height: 50;}\n';
  return css.Parser.parse(cssText);
}

class BorderSample {
  constructor() {
    /** @const @type {!Document} */
    this.document_ = createDoucment();
    /** @const @type {!CSSStyleSheet} */
    this.styleSheet_ = createStyleSheet();
    /** @type {!VisualWindow} */
    this.window_ = VisualWindow.newWindow(this.document_, this.styleSheet_);
  }

  run() {
    const shellWindow = Window.focus.parent;
    shellWindow.appendChild(this.window_);
    return this;
  }

  static run() {
    const app = new BorderSample();
    return app.run();
  }
}

samples.BorderSample = BorderSample;
});


var a = new samples.BorderSample();
