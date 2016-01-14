// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'launchpad', function($export) {
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

  /** @const $type {!Array<!Header>} */
  const HEADER_MODEL = [
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
   * @param {!Map<!TextDocument, !Element>} rowMap
   * @return {!Document}
   */
  function createDocumentTree(rowMap) {
    /** @const @type {!Document} */
    const document = new Document();

    const body = document.createElement('body');
    document.appendChild(body);

    // List header
    const head = document.createElement('header');
    body.appendChild(head);
    for (const header of HEADER_MODEL) {
      const cell = document.createElement('headerCell', header.id);
      cell.appendChild(document.createText(header.label));
      cell.style.width = header.width;
      head.appendChild(cell);
    }

    // List body
    const list = document.createElement('list', 'list');
    body.appendChild(list);

    for (const textDocument of TextDocument.list) {
      const state = TextDocumentState.get(textDocument);
      if (!state) {
        console.log('WARNING', 'no TextDocumentState for', textDocument);
        continue;
      }
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
    const rowModel = createRowModel(textDocument);
    const row = document.createElement('row');
    for (const header of HEADER_MODEL) {
      const cell = document.createElement('cell');
      cell.appendChild(document.createText(rowModel[header.id]));
      cell.style.width = header.width;
      row.appendChild(cell);
    }
    return row;
  }

  /**
   * @param {!TextDocument} textDocument
   * $return {!RowModel}
   */
  function createRowModel(textDocument) {
    return {
        name: textDocument.name,
        file: textDocument.fileName,
        save: lastWriteTimeOf(textDocument),
        size: textDocument.length.toString(),
        state: stateStringFor(textDocument),
    };
  }

  /**
   * @param {!TextDocument} document
   * @return {string}
   */
  function lastWriteTimeOf(document) {
    if (document.lastWriteTime.valueOf())
      return document.lastWriteTime.toLocaleString();
    return '-';
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
      /** @private @const @type {!Map<!TextDocument, !Element>} */
      this.rowMap_ = new Map();
      /** @private @const @type {!Document} */
      this.document_ = createDocumentTree(this.rowMap_);
      // TODO(eval1749): We should use |document.getElementById()|.
      const body = this.document.firstChild;
      this.list_ = body.lastChild;
      windows.TextDocumentState.addObserver(this.didChangeState.bind(this));
    }

    /** @return {!Document} */
    get document() { return this.document_; }

    /**
     * @param {!TextDocument} textDocument
     * @param {!TextDocumentState} state
     */
    didChangeState(textDocument, state) {
      const oldRow = this.rowMap_.get(textDocument) || null;
      console.log('state', textDocument, textDocument.length, oldRow);
      if (!oldRow) {
        const newRow = createRow(this.document_, textDocument);
        this.rowMap_.set(textDocument, newRow);
        this.list_.appendChild(newRow);
        return;
      }
      const rowModel = createRowModel(textDocument);
      let cell = oldRow.firstChild;
      for (const header of HEADER_MODEL) {
        const text = /** @type {!Text} */(cell.firstChild);
        text.data = rowModel[header.id];
        cell = cell.nextSibling;
      }
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
        CSSRuleBuilder.selector('body').display('block')
            .fontSize(14)
            .margin(0, 4, 0, 4)
            .padding(0, 4, 0, 4)
            .build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('header').display('inline').build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('headerCell').display('inline').padding(3)
            .border('#888', 0, 0, 0, 1).build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('cell').display('inline').padding(3).build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('list').display('block').build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('row').display('block').build());
    styleSheet.appendRule(
        // TODO(eval1749): We should use "rgba(r, g, b, a)" instead of hex-form.
        CSSRuleBuilder.selector('.hover')
            .backgroundColor('#3399FF10')
            .border('#3399FF30', 1).build());
    return styleSheet;
  }

  class View {
    /**
     * $param {!Model} model
     */
    constructor(model) {
      /** @private @type {Element} */
      this.lastHover_ = null;
      /** @private @type {!CSSStyleSheet} */
      this.styleSheet_ = createStyleSheet();
      /** @private @const @type {!VisualWindow} */
      this.window_ = VisualWindow.newWindow(model.document, this.styleSheet_);

      this.window_.addEventListener('mousemove', this.onMouseMove.bind(this));
    }

    /** @param {!MouseEvent} event */
    onMouseMove(event) {
      let node = this.window_.hitTest(event.clientX, event.clientY);
      while (node) {
        if (node.nodeName === 'row')
          break;
        node = node.parentNode;
      }
      const hover = /** @type {Element} */(node);
      if (hover === this.lastHover_)
        return;
      if (this.lastHover_)
        this.lastHover_.classList.toggle('hover');
      if (hover)
        hover.classList.toggle('hover');
      this.lastHover_ = hover;
    }

    /** @return {!VisualWindow} */
    get window() { return this.window_; }
  }

  $export({Model, View});
});

if (TextDocument.find('*scratch*')) {
  var model = new launchpad.Model();
  var view = new launchpad.View(model);
  var editorWindow = new EditorWindow();
  editorWindow.appendChild(view.window);
  editorWindow.realize();
}
