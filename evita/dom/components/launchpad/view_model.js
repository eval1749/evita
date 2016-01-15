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
  var RowListModel;

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

  // TODO(eval1749): We should move |nodeIndexOf()| to "//evita/dom/visuals".
  /**
   * @param {Node} node
   * @return {number}
   */
  function nodeIndexOf(node) {
    if (!node)
      throw new Error('Null pointer');
    let index = 0;
    for (const runner of node.parentNode.childNodes) {
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
    const row = document.createElement('row');
    row.data = new TextDocumentModel(textDocument, row);
    return row;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // ListModel
  //
  class ListModel {
    constructor() {
      /** @private @const @type {!Map<!TextDocument, !Element>} */
      this.rowMap_ = new Map();
      /** @private @const @type {!Document} */
      this.document_ = createDocumentTree(this.rowMap_);
      /** @private @const $type {!SelectionModel} */
      this.selection_ = new SelectionModel();

      // TODO(eval1749): We should use |document.getElementById()|.
      const body = this.document.firstChild;
      this.list_ = body.lastChild;

      TextDocument.addObserver(this.didChangeDocuments.bind(this));
    }

    /** @return {!Document} */
    get document() { return this.document_; }

    /** $return {!SelectionModel} */
    get selection() { return this.selection_; }

    /**
     * @private
     * @param {!TextDocument} textDocument
     */
    didAddTextDocument(textDocument) {
      const newRow = createRow(this.document_, textDocument);
      this.rowMap_.set(textDocument, newRow);
      this.list_.appendChild(newRow);
      this.selection_.didAddNode(newRow);
    }

    /**
     * @private
     * @param {string} action
     * @param {!TextDocument} textDocument
     */
    didChangeDocuments(action, textDocument) {
      switch (action) {
        case 'add':
          return this.didAddTextDocument(textDocument);
        case 'remove':
          return this.didRemoveTextDocument(textDocument);
      }
    }

    /**
     * @private
     * @param {!TextDocument} textDocument
     */
    didRemoveTextDocument(textDocument) {
      const oldRow = this.rowMap_.get(textDocument);
      this.rowMap_.delete(textDocument);
      oldRow.parentNode.removeChild(oldRow);
      this.selection_.didRemoveNode(oldRow);
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
      for (let runner = this.anchorNode_; runner !== this.focusNode_;
           runner = runner.previousSibling) {
        this.selectedNodes_.add(/** @type {!Node} */ (runner));
      }
    }

    /** @return {!Generator<!Node>} */
    * items() {
      if (!this.anchorNode_)
        return;
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

      this.observer_ =
          new TextMutationObserver(this.mutationCallback.bind(this));

      this.document_.addEventListener(Event.Names.BEFORELOAD,
                                      this.willLoadTextDocument.bind(this));
      this.document_.addEventListener(Event.Names.LOAD,
                                      this.didLoadTextDocument.bind(this));
    }

    /** @return {!TextDocument} */
    get document() { return this.document_; }

    close() { this.observer_.disconnect(); }

    /**
     * @private
     * $return {!RowListModel}
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
      const document = this.document_;
      const numberOfWindows = document.listWindows().length;
      return [
        document.modified ? '*' : '-',
        document.readonly ? '%' : '-',
        document.fileName ? OBSOLETE_MARK_MAP.get(document.obsolete) : '-',
        (count => count > 9 ? 'm' : (count || '-'))(numberOfWindows),
      ].join('');
    }

    /** @private */
    setupRow() {
      const rowModel = this.computeRowModel();
      const document = this.row_.ownerDocument;
      for (const header of HEADER_MODEL) {
        const cell = document.createElement('cell');
        cell.appendChild(document.createText(rowModel[header.id]));
        cell.style.width = header.width;
        this.row_.appendChild(cell);
      }
    }

    /** @private */
    update() {
      const rowModel = this.computeRowModel();
      let cell = this.row_.firstChild;
      for (const header of HEADER_MODEL) {
        const text = /** @type {!Text} */ (cell.firstChild);
        text.data = rowModel[header.id];
        cell = cell.nextSibling;
      }
    }

    ////////////////////////////////////////////////////////////
    //
    // Callbacks
    //

    /** @private */
    didLoadTextDocument() {
      this.saved_ = '';
      this.update();
    }

    /**
     * @private
     * @param {!Array.<!TextMutationRecord>} mutations
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
    const styleSheet = new CSSStyleSheet();
    styleSheet.appendRule(CSSRuleBuilder.selector('body')
                              .display('block')
                              .fontSize(13)
                              .margin(0, 4, 0, 4)
                              .padding(0, 4, 0, 4)
                              .build());
    // TODO(eval1749): We should set text trimming for cells.
    styleSheet.appendRule(
        CSSRuleBuilder.selector('header').display('inline').build());
    styleSheet.appendRule(CSSRuleBuilder.selector('headerCell')
                              .display('inline')
                              .padding(3)
                              .border('#888', 0, 0, 0, 1)
                              .build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('cell').display('inline').padding(3).build());
    styleSheet.appendRule(
        CSSRuleBuilder.selector('list').display('block').build());
    styleSheet.appendRule(CSSRuleBuilder.selector('row')
                              .display('block')
                              .border('#fff', 1)
                              .build());
    styleSheet.appendRule(
        // TODO(eval1749): We should use "rgba(r, g, b, a)" instead of hex-form.
        CSSRuleBuilder.selector('.hover')
            .backgroundColor('#3399FF10')
            .border('#3399FF30', 1)
            .build());
    styleSheet.appendRule(
        // TODO(eval1749): We should use "rgba(r, g, b, a)" instead of hex-form.
        CSSRuleBuilder.selector('.inactiveSelected')
            .backgroundColor('#BFCDBF33')  // rgba(191, 205, 191, 0.2)
            .border('#BFCDBF', 1)
            .build());
    styleSheet.appendRule(
        // TODO(eval1749): We should use "rgba(r, g, b, a)" instead of hex-form.
        CSSRuleBuilder.selector('.activeSelected')
            .backgroundColor('#3399FF33')  // rgba(51, 153, 255, 0.2)
            .border('#3399FF30', 1)
            .build());
    return styleSheet;
  }

  class View {
    /**
     * $param {!ListModel} model
     */
    constructor() {
      /** @private @type {boolean} */
      this.hasFocus_ = true;
      /** @private @type {Element} */
      this.lastHover_ = null;
      /** @private $type {!ListModel} */
      this.model_ = new ListModel();
      /** @private @type {!CSSStyleSheet} */
      this.styleSheet_ = createStyleSheet();
      /** @private @const @type {!VisualWindow} */
      this.window_ =
          VisualWindow.newWindow(this.model_.document, this.styleSheet_);

      /** @private @const @type {!Element} */
      this.list_ = /** @type {!Element} */ (
          this.model_.document.firstChild.firstChild.nextSibling);

      /** @private $type {!SelectionModel} */
      this.selection_ = this.model_.selection;

      this.selection_.collapseTo(
          /** @type {!Element} */ (this.list_.firstChild));
      this.updateSelected();

      this.setTabData();

      this.window_.addEventListener(Event.Names.BLUR, this.onBlur.bind(this));
      this.window_.addEventListener(Event.Names.DBLCLICK,
                                    this.onDoubleClick.bind(this));
      this.window_.addEventListener(Event.Names.FOCUS, this.onFocus.bind(this));
      this.window_.addEventListener(Event.Names.KEYDOWN,
                                    this.onKeyDown.bind(this));
      this.window_.addEventListener(Event.Names.MOUSEDOWN,
                                    this.onMouseDown.bind(this));
      this.window_.addEventListener(Event.Names.MOUSELEAVE,
                                    this.onMouseLeave.bind(this));
      this.window_.addEventListener(Event.Names.MOUSEMOVE,
                                    this.onMouseMove.bind(this));
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
      let node = this.window_.hitTest(pointX, pointY);
      while (node) {
        if (node.nodeName === 'row')
          break;
        node = node.parentNode;
      }
      return /** @type {Element} */ (node);
    }

    openSelected() {
      for (const row of this.selection_.items()) {
        const model = /** @type {!TextDocument} */ (row.data);
        windows.activate(this.window_, model.document);
      }
    }

    /** @private */
    setTabData() {
      const tabData = new TabData();
      tabData.icon = 0;
      tabData.state = 0;
      tabData.title = 'TextDocument List';
      tabData.tooltip = '';
      Editor.setTabData(this.window_, tabData);
    }

    updateSelected() {
      const add = this.hasFocus_ ? 'activeSelected' : 'inactiveSelected';
      const remove = this.hasFocus_ ? 'inactiveSelected' : 'activeSelected';
      for (const row of this.list_.childNodes) {
        row.classList.remove(remove);
        if (this.selection_.contains(row)) {
          row.classList.add(add);
          continue;
        }
        row.classList.remove(add);
      }
    }

    ////////////////////////////////////////////////////////////
    //
    // Event handlers
    //
    /** @private */
    onBlur() {
      if (this.lastHover_) {
        this.lastHover_.classList.remove('hover');
        this.lastHover_ = null;
      }
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
      this.lastHover_ = null;
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
      console.log('onKeyDown', event.keyCode.toString(16));
    }

    /** @private @param {!MouseEvent} event */
    onMouseDown(event) {
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

    /** @private @param {!MouseEvent} event */
    onMouseLeave(event) {
      if (!this.lastHover_)
        return;
      this.lastHover_.classList.remove('hover');
      this.lastHover_ = null;
    }

    /** @private @param {!MouseEvent} event */
    onMouseMove(event) {
      const hover = this.findRowAtPoint(event.clientX, event.clientY);
      if (hover === this.lastHover_)
        return;
      if (this.lastHover_)
        this.lastHover_.classList.remove('hover');
      if (hover)
        hover.classList.add('hover');
      this.lastHover_ = hover;
    }
  }

  $export({View});
});

if (TextDocument.find('*scratch*')) {
  var view = new launchpad.View();
  var editorWindow = new EditorWindow();
  editorWindow.appendChild(view.window);
  editorWindow.realize();
}
