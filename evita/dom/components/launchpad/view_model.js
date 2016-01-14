// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'launchpad', function($export) {
  //////////////////////////////////////////////////////////////////////
  //
  // CSSRuleBuilder
  //
  // TODO(eval1749): We should move |CSSRuleBuilder| to "evita/dom/visuals/".
  class CSSRuleBuilder {
    /**
     * @param {string} selectorText
     */
    constructor(selectorText) {
      /** @private @const @type {string} */
      this.selectorText_ = selectorText;

      /** @private @const @type {!CSSStyleDeclaration} */
      this.style_ = new CSSStyleDeclaration();
    }

    /**
     * @param {number} width
     * @param {string} color
     * $return {!CSSRuleBuilder}
     */
    border(width, color) {
      this.style_.borderBottomColor = color;
      this.style_.borderLeftColor = color;
      this.style_.borderRightColor = color;
      this.style_.borderTopColor = color;

      const width_string = width.toString();
      this.style_.borderBottomWidth = width_string;;
      this.style_.borderLeftWidth = width_string;;
      this.style_.borderRightWidth = width_string;;
      this.style_.borderTopWidth = width_string;;
      return this;
    }

    /** @return {!CSSStyleRule} */
    build() {
      return new CSSStyleRule(this.selectorText_, this.style_);
    }

    /**
     * @param {string} display
     * $return {!CSSRuleBuilder}
     */
    display(display) {
      this.style_.display = display;
      return this;
    }

    /**
     * @param {number} width
     * $return {!CSSRuleBuilder}
     */
    padding(width) {
      const width_string = width.toString();
      this.style_.paddingBottom = width_string;
      this.style_.paddingLeft = width_string;
      this.style_.paddingRight = width_string;
      this.style_.paddingTop = width_string;
      return this;
    }

    /**
     * @param {string} selectorText
     * $return {!CSSRuleBuilder}
     */
    static selector(selectorText) {
      return new CSSRuleBuilder(selectorText);
    }
  }

  /**
   * @typedef {{
   *    align: string,
   *    id: string,
   *    label: string,
   *    width: number,
   * }}
   */
  var Header;

  /** @const $type {!Array<!Header>} */
  const HEADERS = [
    {id: 'name', label: 'Name', width: 300, align: 'left'},
    {id: 'size', label: 'Size', width: 50, align: 'right'},
    {id: 'state', label: 'State', width: 50, align: 'left'},
    {id: 'save', label: 'Saved At', width: 200, align: 'left'},
    {id: 'file', label: 'File', width: 200, align: 'left'}
  ];

  /** @type {!Map.<!symbol, string>} */
  const OBSOLETE_MARK_MAP = new Map([
    [TextDocument.Obsolete.NO, '-'],
    [TextDocument.Obsolete.CHECKING, '.'],
    [TextDocument.Obsolete.IGNORE, '%'],
    [TextDocument.Obsolete.UNKNOWN, '?'],
    [TextDocument.Obsolete.YES, '*']
  ]);

  const TextDocumentState = windows.TextDocumentState;

  /**
   * @return {!Document}
   */
  function createDocumentTree() {
    /** @const @type {!Document} */
    const document = new Document();

    const body = document.createElement('body');
    document.appendChild(body);

    // List header
    const head = document.createElement('header');
    body.appendChild(head);
    for (const header of HEADERS)
      head.appendChild(document.createElement('headerCell', header.id));

    // List body
    const list = document.createElement('list');
    body.appendChild(list);

    for (const textDocument of TextDocument.list) {
      const state = TextDocumentState.get(textDocument);
      if (!state) {
        console.log('WARNING', 'no TextDocumentState for', textDocument);
        continue;
      }
      list.appendChild(createRow(document, state));
    }

    return document;
  }

  /**
   * @param {!Document} document
   * @param {!TextDocumentState} state
   * @return {!Element}
   */
  function createRow(document, state) {
    const row = document.createElement('row');

    const textDocument = state.document_;

    const nameCell = document.createElement('cell', 'name');
    nameCell.appendChild(document.createText(state.name));
    row.appendChild(nameCell);

    const sizeCell = document.createElement('cell', 'size');
    sizeCell.appendChild(
        document.createText(textDocument.length.toString()));
    row.appendChild(sizeCell);

    const stateCell = document.createElement('cell', 'state');
    stateCell.appendChild(document.createText(stateStringFor(textDocument)));
    row.appendChild(stateCell);

    const saveCell = document.createElement('cell', 'save');
    saveCell.appendChild(
        document.createText(textDocument.lastWriteTime.valueOf()
            ? textDocument.lastWriteTime.toLocaleString() : '-'));
    row.appendChild(saveCell);

    return row;
  }

  /**
   * @param {!TextDocument} document
   * @return {!string}
   */
  function stateStringFor(document) {
    const numberOfWindows = document.listWindows().length;
    return [
      document.modified ? '*' : '-',
      document.readonly ? '%' : '-',
      document.fileName ? OBSOLETE_MARK_MAP.get(document.obsolete) : '-',
      (count => count > 9 ? 'm' : (count || '-'))(numberOfWindows),
    ].join('');
  }

  //////////////////////////////////////////////////////////////////////
  //
  // Model
  //
  class Model {
    constructor() {
      /** @private @const @type {!Document} */
      this.document_ = createDocumentTree();
      windows.TextDocumentState.addObserver(this.didChangeState.bind(this));
    }

    /** @return {!Document} */
    get document() { return this.document_; }

    /**
     * @param {!TextDocumentState} state
     */
    didChangeState(state) {
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // View
  //

  /**
   * @return {!CSSStyleSheet}
   */
  function createStyleSheet() {
    const styleSheet = new CSSStyleSheet();
    styleSheet.appendRule(
        CSSRuleBuilder.selector('body').display('block').build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('header').display('inline').padding(2).build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('headerCell').display('block').padding(2)
            .build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('cell').display('inline').padding(2)
            .border(1, '#888').build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('list').display('block').border(1, '#0f0').build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('row').display('block').border(1, '#00f').build());
    styleSheet.appendRule(
        // TODO(eval1749): We should use "rgba(r, g, b, a)" instead of hex-form.
        CSSRuleBuilder.selector('.hover').border(1, '#3399FF20').build());
    return styleSheet;
  }

  class View {
    /**
     * $param {!Model} model
     */
    constructor(model) {
      /** @private @const @type {!VisualWindow} */
      this.window_ = VisualWindow.newWindow(model.document, createStyleSheet());
    }

    /** @return {!VisualWindow} */
    get window() { return this.window_; }
  }

  $export({Model, View});
});

var model = new launchpad.Model();
var view = new launchpad.View(model);
var editorWindow = new EditorWindow();
editorWindow.appendChild(view.window);
editorWindow.realize();
