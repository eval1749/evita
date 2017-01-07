// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('launchpad');

goog.require('base');
goog.require('css');
goog.require('imaging');
goog.require('visuals');
goog.require('windows');

goog.scope(function() {

/** @constructor */
const CSSStyleSheet = css.CSSStyleSheet;

/** @constructor */
const Document = visuals.Document;

/** @constructor */
const Element = visuals.Element;

/** @constructor */
const Image = visuals.Image;

/** @constructor */
const Node = visuals.Node;

/** @constructor */
const Singleton = base.Singleton;

/** @constructor */
const Text = visuals.Text;

/** @constructor */
const IconUtil = imaging.IconUtil;

/**
 * @typedef {{
 *    align: string,
 *    id: string,
 *    label: string,
 *    width: number,
 * }}
 */
var Header;

/**
 * @typedef {{
 *    name: string,
 *    size: string,
 *    state: string,
 *    save: string,
 * }}
 */
var RowModel;

// "imageres.dll,-2" and "shell32.dll,-1" are a blank document icon.
/** @const @type {!Uint8ClampedArray} 16x16 RGBA pixels */
const kBlankDocumentImageData = new Uint8ClampedArray(base.atob(
    'AAAAAAAAAAClpaX_o6Sk_6Kiov+hoaH_n5+g_56env+cnZ3_m5' +
    'ub_5mamv+dnZ3vlpeXMAAAAAAAAAAAAAAAAAAAAAAAAAAAo6Sk' +
    '__39_f_9_f3__f39__z9_f_8_P3__Pz9__z8_f+mpqb_1tbW_5' +
    'qbm++TlJUwAAAAAAAAAAAAAAAAAAAAAKKiov_9_f3_+vv7__r6' +
    '+__5+vr_+fn6__j5+v_4+fr_pqam_+rq6v_V1dX_l5iZ7wAAAA' +
    'AAAAAAAAAAAAAAAAChoaH__f39__r6+__5+vr_+fr6__n5+v_4' +
    '+fr_+Pn5_6ampv+MjIz_jIyM_5CSkv8AAAAAAAAAAAAAAAAAAA' +
    'AAn5+g__39_f_5+vr_+fr6__n5+v_4+fr_+Pn6__j5+f_4+Pn_' +
    '9_j5__v7_P+PkJH_AAAAAAAAAAAAAAAAAAAAAJ6env_8_f3_+f' +
    'r6__n5+v_4+fr_+Pn6__j5+v_4+Pn_9_j5__f4+f_7+_z_jY+P' +
    '_wAAAAAAAAAAAAAAAAAAAACcnZ3__Pz9__n5+v_4+fr_+Pn6__' +
    'j5+v_4+fn_9_j5__f4+f_3+Pj_+_v8_4yNjv8AAAAAAAAAAAAA' +
    'AAAAAAAAm5ub__z8_f_4+fr_+Pn6__j5+v_4+fn_+Pj5__f4+f' +
    '_3+Pj_9vf4__v7_P+LjI3_AAAAAAAAAAAAAAAAAAAAAJmamv_8' +
    '_P3_+Pn5__j5+f_4+Pn_9_j5__f4+f_3+Pn_9_f4__b3+P_7+_' +
    'z_iYqL_wAAAAAAAAAAAAAAAAAAAACYmJn__Pz8__j4+f_3+Pn_' +
    '9_j5__f4+f_3+Pj_9_f4__b3+P_29_j_+_v7_4iJiv8AAAAAAA' +
    'AAAAAAAAAAAAAAlpeX__v8_P_3+Pn_9_j5__f4+f_3+Pj_9vf4' +
    '__b3+P_29_j_9vb3__r7+_+GiIj_AAAAAAAAAAAAAAAAAAAAAJ' +
    'WWlv_7_Pz_9_f4__f3+P_29_j_9vf4__b3+P_29_j_9fb3__X2' +
    '9__6+_v_hYaH_wAAAAAAAAAAAAAAAAAAAACTlJX_+_v8__b3+P' +
    '_29_j_9vf4__b3+P_29_f_9fb3__X29__19vf_+vv7_4OFhv8A' +
    'AAAAAAAAAAAAAAAAAAAAkpOT__v7_P_29_j_9vf4__b29__19v' +
    'f_9fb3__X29__19vf_9fb3__r7+_+ChIT_AAAAAAAAAAAAAAAA' +
    'AAAAAJCSkv_6+_v_+vv7__r7+__6+_v_+vv7__r7+__6+_v_+v' +
    'v7__r7+__6+_v_gIKD_wAAAAAAAAAAAAAAAAAAAACPkJH_jY+P' +
    '_4yNjv+LjI3_iYqL_4iJiv+GiIj_hYaH_4OFhv+ChIT_gIKD_3' +
    '+Bgv8AAAAAAAAAAA=='.replace(/_/g, '/')));

/**
 * @param {string} fileName
 * @return {string}
 */
function extensionOf(fileName) {
  /** @const @type {number} */
  const lastDot = fileName.lastIndexOf('.');
  if (lastDot < 0)
    return '';
  return fileName.substr(lastDot);
}

//////////////////////////////////////////////////////////////////////
//
// IconMap
//
class IconMap {
  constructor() {
    /** @type {!ImageData} */
    this.blankDocumentIcon_ = new ImageData(kBlankDocumentImageData, 16, 16);

    /** @const @type {!Map<string, !ImageData>} */
    this.iconMap_ = new Map();
  }

  /** @return {!ImageData} */
  get defaultIcon() { return this.blankDocumentIcon_; }

  /** @return {!IconMap} */
  static get instance() { return Singleton.get(IconMap); }

  /**
   * @param {string} fileName
   * @return {!Promise<!ImageData>}
   */
  loadIcon(fileName) {
    /** @const @type {string} */
    const extension = extensionOf(fileName);
    /** @const @type {?ImageData} */
    const present = this.iconMap_.get(extension) || null;
    if (present !== null)
      return Promise.resolve(present);
    return IconUtil.loadIconForExtension(extension).then(maybeImage => {
      /** @const @type {!ImageData} */
      const image = maybeImage || this.blankDocumentIcon_;
      this.iconMap_.set(extension, image);
      return image;
    });
  }
}

/** @const @type {!Array<!Header>} */
const HEADER_MODEL = [
  {id: 'name', label: 'Name', width: 300, align: 'left'},
  {id: 'size', label: 'Size', width: 50, align: 'right'},
  {id: 'state', label: 'State', width: 50, align: 'left'},
  {id: 'save', label: 'Saved At', width: 200, align: 'left'},
  {id: 'file', label: 'File', width: 200, align: 'left'}
];

/** @type {!Map<!symbol, string>} */
const OBSOLETE_MARK_MAP = new Map([
  [TextDocument.Obsolete.NO, '-'], [TextDocument.Obsolete.CHECKING, '.'],
  [TextDocument.Obsolete.IGNORE, '%'], [TextDocument.Obsolete.UNKNOWN, '?'],
  [TextDocument.Obsolete.YES, '*']
]);

/**
 * @param {string} string1
 * @param {string} string2
 * @return {number}
 */
function compareString(string1, string2) {
  return string1.localeCompare(string2, void(0), {sensitivity: 'base'});
}

// TODO(eval1749): We should move |nodeIndexOf()| to "//evita/dom/visuals".
/**
 * @param {Node} node
 * @return {number}
 */
function nodeIndexOf(node) {
  if (!node)
    throw new Error('Null pointer');
  /** @type {number} */
  let index = 0;
  for (/** @type {!Node} */ const runner of node.parentNode.childNodes) {
    if (runner === node)
      return index;
    ++index;
  }
  throw new Error(`${node} isn't in ${node.parentNode}`);
}

/**
 * @param {!Map<!TextDocument, !Element>} rowMap
 * @return {!Document}
 */
function createDocumentTree(rowMap) {
  /** @const @type {!Document} */
  const document = new Document();

  /** @const @type {!Element} */
  const body = document.createElement('body');
  document.appendChild(body);

  // List header
  /** @const @type {!Element} */
  const head = document.createElement('header');
  body.appendChild(head);
  for (const header of HEADER_MODEL) {
    /** @const @type {!Element} */
    const cell = document.createElement('headerCell', header.id);
    cell.appendChild(document.createText(header.label));
    cell.style.width = header.width.toString();
    head.appendChild(cell);
  }

  // List body
  /** @const @type {!Element} */
  const list = document.createElement('list', 'list');
  body.appendChild(list);

  /** @const @type {!Array<!TextDocument>} */
  const textDocuments = TextDocument.list.sort(
      (doc1, doc2) => { return compareString(doc1.name, doc2.name); });

  for (const textDocument of textDocuments) {
    /** @const @type {?TextDocumentState} */
    const state = TextDocumentState.get(textDocument);
    if (!state) {
      console.log('WARNING', 'no TextDocumentState for', textDocument);
      continue;
    }
    /** @const @type {!Element} */
    const row = createRow(document, textDocument);
    rowMap.set(textDocument, row);
    list.appendChild(row);
  }

  return document;
}

/**
 * @param {!Document} document
 * @param {!TextDocument} textDocument
 * @return {!Element}
 */
function createRow(document, textDocument) {
  /** @const @type {!Element} */
  const row = document.createElement('row');
  row.data = new TextDocumentModel(textDocument, row);
  return row;
}

//////////////////////////////////////////////////////////////////////
//
// ListModel
//
class ListModel extends SimpleTextDocumentSetObserver {
  constructor() {
    super();

    /** @private @const @type {!Map<!TextDocument, !Element>} */
    this.rowMap_ = new Map();
    /** @private @const @type {!Document} */
    this.document_ = createDocumentTree(this.rowMap_);
    /** @private @const @type {!SelectionModel} */
    this.selection_ = new SelectionModel();

    // TODO(eval1749): We should use |document.getElementById()|.
    const body = this.document.firstChild;
    this.list_ = body.lastChild;

    TextDocument.addObserver(this);
  }

  /** @return {!Document} */
  get document() { return this.document_; }

  /** @return {!SelectionModel} */
  get selection() { return this.selection_; }

  /**
   * TextDocumentSetObserver
   * @private
   * @param {!TextDocument} textDocument
   */
  didAddTextDocument(textDocument) {
    /** @const @type {!Element} */
    const newRow = createRow(this.document_, textDocument);
    this.rowMap_.set(textDocument, newRow);
    /** @type {boolean} */
    let inserted = false;
    for (/** @const @type {!Node} */ const child of this.list_.childNodes) {
      /** @const @type {!Element} */
      const row = /** @type {!Element} */ (child);
      /** @const @type {!TextDocumentModel} */
      const present = /** @type {!TextDocumentModel} */ (row.data);
      if (compareString(textDocument.name, present.name) < 0) {
        this.list_.insertBefore(newRow, row);
        inserted = true;
        break;
      }
    }
    if (!inserted)
      this.list_.appendChild(newRow);
    this.selection_.didAddNode(newRow);
  }

  /**
   * TextDocumentSetObserver
   * @private
   * @param {!TextDocument} textDocument
   */
  didRemoveTextDocument(textDocument) {
    /** @const @type {!Element} */
    const oldRow = /** @type {!Element} */ (this.rowMap_.get(textDocument));
    this.rowMap_.delete(textDocument);
    oldRow.parentNode.removeChild(oldRow);
    this.selection_.didRemoveNode(oldRow);
    /** @const @type {!TextDocumentModel} */
    const textDocumentModel = /** @type {!TextDocumentModel} */ (oldRow.data);
    textDocumentModel.close();
  }
}

//////////////////////////////////////////////////////////////////////
//
// SelectionModel
//
class SelectionModel {
  constructor() {
    /** @private @type {Node} */
    this.anchorNode_ = null;
    /** @private @type {Node} */
    this.focusNode_ = null;
    /** @private @type {!Set<!Node>} */
    this.selectedNodes_ = new Set();
  }

  clear() {
    this.anchorNode_ = this.focusNode_ = null;
    this.selectedNodes_.clear();
  }

  /** @param {!Node} node */
  collapseTo(node) {
    this.anchorNode_ = this.focusNode_ = node;
    this.selectedNodes_.clear();
    this.selectedNodes_.add(node);
  }

  /** @return {boolean} */
  contains(node) { return this.selectedNodes_.has(node); }

  /** @param {!Node} node */
  didAddNode(node) {
    // Nothing to do
  }

  /** @param {!Node} node */
  didRemoveNode(node) { this.unselect(node); }

  extendBackward() {
    if (!this.focusNode_)
      return;
    const previousSibling = this.focusNode_.previousSibling;
    if (!previousSibling)
      return;
    this.extendTo(previousSibling);
  }

  extendForward() {
    if (!this.focusNode_)
      return;
    const nextSibling = this.focusNode_.nextSibling;
    if (!nextSibling)
      return;
    this.extendTo(nextSibling);
  }

  /** @param {!Node} node */
  extendTo(node) {
    if (!this.anchorNode_)
      return this.collapseTo(node);
    this.selectedNodes_.clear();
    this.focusNode_ = node;
    this.selectedNodes_.add(node);
    if (nodeIndexOf(this.anchorNode_) < nodeIndexOf(this.focusNode_)) {
      for (let runner = this.anchorNode_; runner !== this.focusNode_;
           runner = runner.nextSibling) {
        this.selectedNodes_.add(/** @type {!Node} */ (runner));
      }
      return;
    }
    for (/** @type {?Node} */ let runner = this.anchorNode_;
         runner !== this.focusNode_; runner = runner.previousSibling) {
      this.selectedNodes_.add(/** @type {!Node} */ (runner));
    }
  }

  /** @return {!Generator<!Node>} */
  * items() {
    if (!this.anchorNode_)
      return;
    /** @const @type {!Node} */
    const list = this.anchorNode_.parentNode;
    for (const runner of list.childNodes) {
      if (this.selectedNodes_.has(runner))
        yield runner;
    }
  }

  moveBackward() {
    if (!this.focusNode_)
      return;
    this.collapseTo(this.focusNode_.previousSibling || this.focusNode_);
  }

  moveForward() {
    if (!this.focusNode_)
      return;
    this.collapseTo(this.focusNode_.nextSibling || this.focusNode_);
  }

  /** @param {!Node} node */
  toggle(node) {
    if (this.selectedNodes_.has(node))
      return this.unselect(node);
    this.anchorNode_ = this.focusNode_ = node;
    this.selectedNodes_.add(node);
  }

  /** @param {!Node} node */
  unselect(node) {
    this.selectedNodes_.delete(node);
    if (this.anchorNode_ !== node && this.focusNode_ !== node)
      return;
    if (this.anchorNode_ === node) {
      this.anchorNode_ = this.focusNode_;
      return;
    }
    if (this.focusNode_ === node) {
      this.focusNode_ = this.anchorNode_;
      return;
    }
  }
}

//////////////////////////////////////////////////////////////////////
//
// TextDocumentModel
//
class TextDocumentModel {
  /**
   * @param {!TextDocument} textDocument
   * @param {!Element} row
   */
  constructor(textDocument, row) {
    /** @private @const @type {!TextDocument} */
    this.document_ = textDocument;
    /** @private @const @type {!Element} */
    this.row_ = row;
    /** @private @type {string} */
    this.saved_ = '';

    this.setupRow();

    this.observer_ = new TextMutationObserver(this.mutationCallback.bind(this));
    this.observer_.observe(textDocument, {summary: true});

    this.document_.addEventListener(
        Event.Names.BEFORELOAD, this.willLoadTextDocument.bind(this));
    this.document_.addEventListener(
        Event.Names.ATTACH, this.didAttachWindow.bind(this));
    this.document_.addEventListener(
        Event.Names.DETACH, this.didDetachWindow.bind(this));
    this.document_.addEventListener(
        Event.Names.LOAD, this.didLoadTextDocument.bind(this));
  }

  /** @return {!TextDocument} */
  get document() { return this.document_; }

  /** @return {string} */
  get name() { return this.document_.name; }

  close() { this.observer_.disconnect(); }

  /**
   * @private
   * @return {!RowModel}
   */
  computeRowModel() {
    return {
      name: this.document_.name,
      file: this.document_.fileName,
      save: this.computeSaveCell(),
      size: this.document_.length.toString(),
      state: this.computeStateCell(),
    };
  }

  /**
   * @return {string}
   */
  computeSaveCell() {
    if (this.saved_ !== '')
      return this.saved_;
    if (this.document_.lastWriteTime.valueOf())
      return this.document_.lastWriteTime.toLocaleString();
    return '-';
  }

  /**
   * @return {string}
   */
  computeStateCell() {
    /** @const @type {!TextDocument} */
    const document = this.document_;
    /** @const @type {!number} */
    const numberOfWindows = document.listWindows().length;
    return [
      document.modified ? '*' : '-',
      document.readonly ? '%' : '-',
      document.fileName ? OBSOLETE_MARK_MAP.get(document.obsolete) : '-',
      (count => count > 9 ? 'm' : (count || '-'))(numberOfWindows),
    ].join('');
  }

  /**
   * @param {!Document} document
   * @param {string} name
   * @return {!Element}
   */
  createIcon(document, name) {
    /** @const @type {!Element} */
    const icon = document.createElement('icon');
    /** @const @type {!Image} */
    const image = document.createImage(IconMap.instance.defaultIcon);
    IconMap.instance.loadIcon(name).then(imageData => {
      if (imageData === null)
        return;
      image.data = imageData;
    });
    icon.appendChild(image);
    return icon;
  }

  /** @private */
  setupRow() {
    /** @const @type {!RowModel} */
    const rowModel = this.computeRowModel();
    /** @const @type {!Document} */
    const document = this.row_.ownerDocument;
    for (/** @type {!Header} */ const header of HEADER_MODEL) {
      /** @const @type {!Element} */
      const cell = document.createElement('cell');
      /** @const @type {string} */
      const text = rowModel[header.id];
      if (header.id === 'name')
        cell.appendChild(this.createIcon(document, text));
      cell.appendChild(document.createText(text));
      cell.style.width = header.width.toString();
      this.row_.appendChild(cell);
    }
  }

  /** @private */
  update() {
    const rowModel = this.computeRowModel();
    /** @type {!Node} */
    let cell = /** @type {!Node} */ (this.row_.firstChild);
    for (/** @type {!Header} */ const header of HEADER_MODEL) {
      const text = /** @type {!Text} */ (cell.firstChild);
      text.data = rowModel[header.id];
      cell = /** @type {!Node} */ (cell.nextSibling);
    }
  }

  ////////////////////////////////////////////////////////////
  //
  // Callbacks
  //

  /** @private */
  didAttachWindow() { this.update(); }

  /** @private */
  didDetachWindow() { this.update(); }

  /** @private */
  didLoadTextDocument() {
    this.saved_ = '';
    this.update();
  }

  /**
   * @private
   * @param {!Array<!TextMutationRecord>} mutations
   * @param {!TextMutationObserver} observer
   */
  mutationCallback(mutations, observer) { this.update(); }

  /** @private */
  willLoadTextDocument() { this.saved_ = 'loading...'; }
}

//////////////////////////////////////////////////////////////////////
//
// View
//

/**
 * @return {!CSSStyleSheet}
 */
function createStyleSheet() {
  // TODO(eval1749): We should move |cssText| to string resource.
  /** @const @type {string} */
  const cssText = 'body {\n' +
      '  display: block;\n' +
      '  font-size: 13;\n' +
      '  margin: 0 4 0 4;\n' +
      '  padding: 0 4 0 4;\n' +
      '}\n' +
      'header {\n' +
      '  display: inline;\n' +
      '}\n' +
      'headerCell {\n' +
      ' display: inline;\n' +
      ' padding: 3;\n' +
      ' border: #888 0 0 0 1;\n' +
      '}\n' +
      'cell {\n' +
      '  display: inline;\n' +
      '  padding: 3;\n' +
      '}\n' +
      'icon {\n' +
      '  height: 20;\n' +
      '  width: 20;\n' +
      '}\n' +
      'list {\n' +
      '  display: block;\n' +
      '}\n' +
      'row {\n' +
      '  display: block;\n' +
      '  border: solid #fff 1;\n' +
      '}\n' +
      '.inactiveSelected {\n' +
      '  background-color: #BFCDBF4C;  /* rgba(191, 205, 191, 0.3) */\n' +
      '  border: solid #BFCDBF 1\n' +
      '}\n' +
      '.activeSelected {\n' +
      '  background-color: #3399FF4C; /* rgba(51, 153, 255, 0.3) */\n' +
      '  border: solid #3399FF 1;\n' +
      '}\n' +
      'row:hover {\n' +
      '  background-color: #3399FF33;\n' +
      '  border: solid #3399FF33 1;\n' +
      '}';
  return css.Parser.parse(cssText);
}

/** @const @type {!CSSStyleSheet} */
const styleSheet = createStyleSheet();

//////////////////////////////////////////////////////////////////////
//
// LaunchPad
//
class LaunchPad {
  constructor() {
    /** @private @type {boolean} */
    this.hasFocus_ = true;
    /** @private @type {!ListModel} */
    this.model_ = new ListModel();
    /** @private @const @type {!VisualWindow} */
    this.window_ = VisualWindow.newWindow(this.model_.document, styleSheet);

    /** @private @const @type {!Element} */
    this.list_ = /** @type {!Element} */ (
        this.model_.document.firstChild.firstChild.nextSibling);

    /** @private @type {!SelectionModel} */
    this.selection_ = this.model_.selection;

    this.selection_.collapseTo(
        /** @type {!Element} */ (this.list_.firstChild));
    this.updateSelected();

    this.setTabData();

    this.window_.addEventListener(Event.Names.BLUR, this.onBlur.bind(this));
    this.window_.addEventListener(
        Event.Names.DBLCLICK, this.onDoubleClick.bind(this));
    this.window_.addEventListener(Event.Names.FOCUS, this.onFocus.bind(this));
    this.window_.addEventListener(
        Event.Names.KEYDOWN, this.onKeyDown.bind(this));
    this.window_.addEventListener(
        Event.Names.MOUSEDOWN, this.onMouseDown.bind(this));
  }

  /** @return {!VisualWindow} */
  get window() { return this.window_; }

  /**
   * @private
   * @param {number} pointX
   * @param {number} pointY
   * @return {Element}
   */
  findRowAtPoint(pointX, pointY) {
    /** @type {?Node} */
    let runner = this.window_.hitTest(pointX, pointY);
    while (runner) {
      if (runner.nodeName === 'row')
        break;
      runner = runner.parentNode;
    }
    return /** @type {Element} */ (runner);
  }

  openSelected() {
    for (const row of this.selection_.items()) {
      const model = /** @type {!TextDocumentModel} */ (row.data);
      windows.activate(this.window_, model.document);
    }
  }

  /** @private */
  setTabData() {
    /** @const @type {!TabData} */
    const tabData = new TabData();
    tabData.icon = 0;
    tabData.state = 0;
    tabData.title = 'TextDocument List';
    tabData.tooltip = '';
    Editor.setTabData(this.window_, tabData);
  }

  updateSelected() {
    /** @const @type {string} */
    const add = this.hasFocus_ ? 'activeSelected' : 'inactiveSelected';
    /** @const @type {string} */
    const remove = this.hasFocus_ ? 'inactiveSelected' : 'activeSelected';
    /** @type {number} */
    let count = 0;
    for (const child of this.list_.childNodes) {
      /** @const @type {!Element} */
      const row = /** @type {!Element} */ (child);
      row.classList.remove(remove);
      if (this.selection_.contains(row)) {
        row.classList.add(add);
        ++count;
        continue;
      }
      row.classList.remove(add);
    }
    if (!this.window_.parent)
      return;
    this.window_.parent.setStatusBar([`${count} selected`]);
  }

  ////////////////////////////////////////////////////////////
  //
  // Event handlers
  //
  /** @private */
  onBlur() {
    this.hasFocus_ = false;
    this.updateSelected();
  }

  /** @private @param {!MouseEvent} event */
  onDoubleClick(event) {
    const row = this.findRowAtPoint(event.clientX, event.clientY);
    if (!row)
      return;
    this.selection_.collapseTo(row);
    const model = /** @type {!TextDocumentModel} */ (row.data);
    windows.activate(this.window_, model.document);
  }

  /** @private */
  onFocus() {
    this.hasFocus_ = true;
    this.updateSelected();
  }

  /** @private @param {!KeyboardEvent} event */
  onKeyDown(event) {
    switch (event.keyCode) {
      case 0x10D:  // Enter
      case 0x20:   // Space
        this.openSelected();
        return;
      case 0x126:  // ArrowUp
        this.selection_.moveBackward();
        this.updateSelected();
        return;
      case 0x128:  // ArrowDown
        this.selection_.moveForward();
        this.updateSelected();
        return;
      case 0x526:  // ArrowUp
        this.selection_.extendBackward();
        this.updateSelected();
        return;
      case 0x528:  // Shift+ArrowDown
        this.selection_.extendForward();
        this.updateSelected();
        return;
    }
    Window.handleEvent.call(this, event);
  }

  /** @private @param {!MouseEvent} event */
  onMouseDown(event) {
    /** @type {?Element} */
    const row = this.findRowAtPoint(event.clientX, event.clientY);
    if (!row)
      return;
    if (event.shiftKey)
      this.selection_.extendTo(row);
    else if (event.ctrlKey)
      this.selection_.toggle(row);
    else
      this.selection_.collapseTo(row);
    this.updateSelected();
  }
}

/**
 * @this {!Window}
 */
function listTextDocumentCommand() {
  /** @const {?Window} */
  const present = this.parent.children.find(function(child) {
    if (!(child instanceof VisualWindow))
      return false;
    /** @const {!VisualWindow} */
    const window = /** @type {!VisualWindow} */ (child);
    return window.styleSheet == styleSheet;
  }) ||
      null;
  if (present) {
    present.focus();
    return;
  }
  this.parent.appendChild((new LaunchPad().window));
}

Editor.bindKey(Window, 'Ctrl+B', listTextDocumentCommand);

/** @constructor */
launchpad.LaunchPad = LaunchPad;
});
