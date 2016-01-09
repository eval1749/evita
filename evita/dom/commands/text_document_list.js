// Copyright (c) 2014 Project Vogue. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

(function() {
  'use strict';

  /**
   * @type {?TextDocument}
   */
  let documentListTextDocument = null;

  /**
   * @param {!TableSelection} selection
   * @param {number} stateMask
   * @return {!Map.<string, number>}
   */
  function queryRows(selection, stateMask) {
    const keys = TextDocument.list.map(function(document) {
      return document.name;
    });
    const resultSet = new Map();
    selection.getRowStates(keys).forEach(function(state, index) {
      if (state & stateMask)
        resultSet.set(keys[index], state);
    });
    return resultSet;
  }

  /**
   * @this {!TableWindow}
   */
  function closeSelectedTextDocuments() {
    const selection = /** @type{!TableSelection}*/(this.selection);
    const resultSet = queryRows(selection, TableViewRowState.SELECTED);
    let needUpdate = false;
    for (let name of resultSet.keys()) {
      let document = TextDocument.find(name);
      if (!document)
        return;
      document.close();
      // TODO(eval1749): We should handle TextDocument event rather than using
      // |needUpdate| variable. Because |close()| is asynchronous operation
      // and document may not be closed yet or canceled.
      needUpdate = true;
    }
    if (needUpdate)
      ensureTextDocumentList();
  }

  /**
   * @this {!TableWindow}
   */
  function openSelectedTextDocuments() {
    const selection = /** @type{!TableSelection}*/(this.selection);
    const resultSet = queryRows(selection, TableViewRowState.SELECTED);
    const parent = /** @type {!Window} */(this.parent);
    let openCount = 0;
    for (let name of resultSet.keys()) {
      const document = TextDocument.find(name);
      if (!document)
        return;
      if (!openCount)
        windows.activate(parent, document);
      else
        windows.newEditorWindow(document);
      ++openCount;
    }
    if (openCount)
      ensureTextDocumentList();
  }

  /**
   * @return {!TextDocument}
   */
  function ensureTextDocumentList() {
    if (documentListTextDocument)
      return documentListTextDocument;
    const document = TextDocument.new('*document list*');
    document.bindKey('Delete', closeSelectedTextDocuments);
    document.bindKey('Enter', openSelectedTextDocuments);
    documentListTextDocument = document;
    TextDocumentState.addObserver(function(document, state) {
      if (document === documentListTextDocument)
        return;
      updateTextDocumentList();
    });
    updateTextDocumentList();
    return document;
  }

  /** @type {!Map.<!symbol, string>} */
  const OBSOLETE_MARK_MAP = new Map([
    [TextDocument.Obsolete.NO, '-'],
    [TextDocument.Obsolete.CHECKING, '.'],
    [TextDocument.Obsolete.IGNORE, '%'],
    [TextDocument.Obsolete.UNKNOWN, '?'],
    [TextDocument.Obsolete.YES, '*']
  ]);

  function updateTextDocumentList() {
    const documentWindowCountMap = new Map();
    EditorWindow.list.forEach(function(editorWindow) {
      editorWindow.children.forEach(function(window) {
        if (!(window instanceof TextDocumentWindow))
          return;
        const document = window.document;
        const count = documentWindowCountMap.get(document);
        documentWindowCountMap.set(document, count ?  count + 1 : 1);
      });
    });

    /**
     * @param {!TextDocument} document
     * @return {!string}
     */
    function stateString(document) {
      return [
        document.modified ? '*' : '-',
        document.readonly ? '%' : '-',
        document.fileName ? OBSOLETE_MARK_MAP.get(document.obsolete) : '-',
        (function(count) { return count > 9 ? 'm' : (count || '-'); })(
            documentWindowCountMap.get(document))
      ].join('');
    }

    const document = ensureTextDocumentList();
    const range = new Range(document, 0, document.length);
    range.text = '';
    range.text = 'Name\tSize\tState\t\Saved At\tFile\n';
    TextDocument.list.forEach(function(document) {
      range.collapseTo(range.end);
      const fields = [
        document.name,
        document.length,
        stateString(document),
        document.lastWriteTime.valueOf() ?
            document.lastWriteTime.toLocaleString() : '-',
        document.fileName
      ];
      range.text = fields.join('\t') + '\n';
    });
    return document;
  }

  /**
   * @this {!Window}
   */
  function listTextDocumentCommand() {
    const document = ensureTextDocumentList();
    const window = this.parent.children.find(function(window) {
      if (!(window instanceof TableWindow))
        return false;
      const tableWindow = /** @type(!TableWindow) */(window);
      return tableWindow.document === document;
    });
    if (window) {
      window.focus();
      return;
    }
    const tableWindow = new TableWindow(document);
    const tabData = new TabData();
    tabData.icon = 0;
    tabData.state = 0;
    tabData.title = 'TextDocument List';
    tabData.tooltip = '';
    Editor.setTabData(tableWindow, tabData);
    this.parent.appendChild(tableWindow);
  }

  Editor.bindKey(Window, 'Ctrl+B', listTextDocumentCommand);
})();
