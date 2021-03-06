// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('imaging.iconViewer');

goog.require('css');

goog.scope(function() {

const CSSRuleBuilder = css.CSSRuleBuilder;
const CSSStyleSheet = css.CSSStyleSheet;
const Document = visuals.Document;

/** @return {!Document} */
function createDocument() {
  const document = new Document();
  document.elementIdMap_ = new Map();
  document.getElementById = function(id) {
    return this.elementIdMap_.get(id) || null;
  };

  // body
  const body = document.createElement('body');
  document.appendChild(body);

  // title
  const title = document.createElement('title', 'title');
  document.elementIdMap_.set('title', title);
  body.appendChild(title);
  title.appendChild(document.createText('No file'));

  // icons
  const icons = document.createElement('icons', 'icons');
  body.appendChild(icons);
  document.elementIdMap_.set('icons', icons);

  return document;
}

/** @return {!CSSStyleSheet} */
function createStyleSheet() {
  /** @const @type {string} */
  const cssText = 'body {\n' +
      '  display: block;\n' +
      '  font-size: 13;\n' +
      '  margin: 0 4 0 4;\n' +
      '  padding: 0 4 0 4\n' +
      '}\n' +
      'icon { padding: 2 }\n' +
      'icons { display: block }\n' +
      'iconImage { height: 40; width: 40 }\n' +
      'iconName { display: block }\n' +
      'row { display: block }\n';
  return css.Parser.parse(cssText);
}

//////////////////////////////////////////////////////////////////////
//
// IconViewer
//
class IconViewer {
  constructor() {
    /** @const @type {number} */
    this.columnCount_ = 25;
    this.document_ = createDocument();
    /** @const @type {number} */
    this.iconSize_ = 32;
    this.styleSheet_ = createStyleSheet();
    this.window_ = null;
  }

  /**
   * @public
   * @param {string} fileName
   */
  run(fileName) {
    this.setupDocument(fileName);
    if (this.window_ !== null)
      return;
    this.window_ = VisualWindow.newWindow(this.document_, this.styleSheet_);
    const shellWindow = new EditorWindow();
    shellWindow.appendChild(this.window_);
    shellWindow.realize();
  }

  /**
   * @private
   * @param {string} fileName
   */
  setupDocument(fileName) {
    this.document_.getElementById('title').firstChild.data = fileName;
    const icons = this.document_.getElementById('icons');
    while (icons.firstChild)
      icons.removeChild(icons.firstChild);
    /** @type {WinResource} */
    let maybeResource = null;
    WinResource.open(fileName)
        .then(resource => {
          maybeResource = resource;
          return resource.getResourceNames(WinResource.RT_GROUP_ICON);
        })
        .then(names => {
          const resource = /** @type {!WinResource} */ (maybeResource);
          const promises = [];
          let column = 0;
          let row = null;
          for (const name of names) {
            if (column % this.columnCount_ === 0) {
              row = this.document_.createElement('row');
              icons.appendChild(row);
            }
            const icon = this.document_.createElement('icon');
            row.appendChild(icon);
            ++column;
            promises.push(this.setupIcon(resource, name, icon));
          }
          return Promise.all(promises).then(x => resource);
        })
        .then(resource => { resource.close(); })
        .catch(exception => {
          if (maybeResource)
            maybeResource.close();
          throw exception;
        });
  }

  /**
   * @param {!WinResource} resource
   * @param {string} groupName
   * @param {!Element} icon
   */
  setupIcon(resource, groupName, icon) {
    const groupBuffer = new Uint8Array(1024);
    const iconBuffer = new Uint8Array(1024 * 32);
    const iconImage = this.document_.createElement('iconImage');
    icon.appendChild(iconImage);
    const iconName = this.document_.createElement('iconName');
    icon.appendChild(iconName);
    iconName.appendChild(this.document_.createText(groupName));
    return resource.load(WinResource.RT_GROUP_ICON, groupName, groupBuffer)
        .then(dataSize => {
          const iconId = WinResource.lookupIcon(groupBuffer, this.iconSize_);
          return resource.load(WinResource.RT_ICON, `#${iconId}`, iconBuffer);
        })
        .then(imageSize => {
          const imageData = ImageData.decode(
              'image/ico;size=32', iconBuffer.subarray(0, imageSize));
          const image = this.document_.createImage(imageData);
          iconImage.appendChild(image);
        });
  }

  /**
   * @public
   * @param {string} fileName
   */
  static run(fileName) { return new IconViewer().run(fileName); }
}

// Usage:
// imaging.IconViewer.run('icon file name');

/** @constructor */
imaging.IconViewer = IconViewer;
});
