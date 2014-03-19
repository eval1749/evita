// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  /**
   * @type {?Document}
   */
  var document_list_document = null;

  /**
   * @param {!TableSelection} selection
   * @param {number} state_mask
   * @return {!Object.<string, number>}
   */
  function queryRows(selection, state_mask) {
    var keys = Document.list.map(function(document) {
      return document.name;
    });
    var map = new Object();
    selection.getRowStates(keys).forEach(function(state, index) {
      if (state & state_mask)
        map[keys[index]] = state;
    });
    return map;
  }

  /**
   * @this {TableWindow}
   */
  function closeSelectedDocuments() {
    var result_set = queryRows(this.selection, TableViewRowState.SELECTED);
    var need_update = false;
    Object.keys(result_set).forEach(function(name) {
      var document = Document.find(name);
      if (!document)
        return;
      document.close();
      // TODO(yosi) We should handle Document event rather than using
      // |need_update| variable. Because |close()| is asynchronus operation
      // and document may not be closed yet or cancled.
      need_update = true;
    });
    if (need_update)
      ensureDocumentList();
  }

  /**
   * @this {TableWindow}
   */
  function openSelectedDocuments() {
    var result_set = queryRows(this.selection, TableViewRowState.SELECTED);
    var parent = /** @type {!Window} */(this.parent);
    var open_count = 0;
    Object.keys(result_set).forEach(function(name) {
      var document = Document.find(name);
      if (!document)
        return;

      if (!open_count)
        windows.activate(parent, document);
      else
        windows.newEditorWindow(document);
      ++open_count;
    });
    if (open_count)
      ensureDocumentList();
  }

  /**
   * @return {!Document}
   */
  function ensureDocumentList() {
    if (document_list_document)
      return document_list_document;
    var document = new Document('*document list*');
    document.bindKey('Delete', closeSelectedDocuments);
    document.bindKey('Enter', openSelectedDocuments);
    Document.list.forEach(function(document) {
      document.addEventListener('attach', updateDocumentList);
      document.addEventListener('detach', updateDocumentList);
    });
    Document.addObserver(function(type, document) {
      document.addEventListener('attach', updateDocumentList);
      document.addEventListener('detach', updateDocumentList);
      updateDocumentList();
    });
    document_list_document = document;
    updateDocumentList();
    return document;
  }

  var OBSOLETE_MARK_MAP = {};
  OBSOLETE_MARK_MAP[Document.Obsolete.NO] = '-';
  OBSOLETE_MARK_MAP[Document.Obsolete.CHECKING] = '.';
  OBSOLETE_MARK_MAP[Document.Obsolete.IGNORE] = '%';
  OBSOLETE_MARK_MAP[Document.Obsolete.UNKNOWN] = '?';
  OBSOLETE_MARK_MAP[Document.Obsolete.YES] = '*';

  function updateDocumentList() {
    var document_window_count_map = new Map();
    EditorWindow.list.forEach(function(editorWindow) {
      editorWindow.children.forEach(function(window) {
        if (!(window instanceof DocumentWindow))
          return;
        var document = window.document;
        var count = document_window_count_map.get(document);
        document_window_count_map.set(document, count ?  count + 1 : 1);
      });
    });

    /**
     * @param {!Document} document
     * @return {!string}
     */
    function stateString(document) {
      return [
        document.modified ? '*' : '-',
        document.readonly ? '%' : '-',
        document.filename ? OBSOLETE_MARK_MAP[document.obsolete] : '-',
        (function(count) { return count > 9 ? 'm' : (count || '-'); })(
            document_window_count_map.get(document))
      ].join('');
    }

    var document = ensureDocumentList();
    var range = new Range(document, 0, document.length);
    range.text = '';
    range.text = 'Name\tSize\tState\t\Saved At\tFile\n';
    Document.list.forEach(function(document) {
      range.collapseTo(range.end);
      var fields = [
        document.name,
        document.length,
        stateString(document),
        document.lastWriteTime.valueOf() ?
            document.lastWriteTime.toLocaleString() : '-',
        document.filename
      ];
      range.text = fields.join('\t') + '\n';
    });
    return document;
  }

  /**
   * @this {!Window}
   */
  function listDocumentCommand() {
    var document = ensureDocumentList();
    var window = this.parent.children.find(function(window) {
      if (!(window instanceof TableWindow))
        return false;
      var table_window = /** @type(!TableWindow) */(window);
      return table_window.document == document;
    });
    if (window) {
      window.focus();
      return;
    }
    var table_window = new TableWindow(document);
    Editor.setTabData(table_window, {
      // TODO(yosi) We should use the icon for document list instead of default
      // icon.
      icon: 0,
      state: 0,
      title: 'Document List',
      tooltip: '',
    });
    this.parent.appendChild(table_window);
  }

  Editor.bindKey(Window, 'Ctrl+B', listDocumentCommand);
})();
