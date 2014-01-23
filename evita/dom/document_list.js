// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  // TODO(yosi) We should move |newEditorWindow()| to another place to use
  // other files, e.g. "window_commands.js".
  /**
   * @param {!Document} document
   */
  function newEditorWindow(document) {
    var editor_window = new EditorWindow();
    newTextWindow(editor_window, document);
    editor_window.realize();
  }

  // TODO(yosi) We should move |newTextWindow()| to another place to use
  // other files, e.g. "window_commands.js".
  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  function newTextWindow(parent, document) {
    parent.add(new TextWindow(new Range(document)));
  }

  // TODO(yosi) We should move |openInCurrentParent()| to another place to use
  // other files, e.g. "window_commands.js".
  /**
   * @param {!Window} parent
   * @param {!Document} document
   */
  function openInCurrentParent(parent, document) {
    var present = parent.children.find(function(window) {
      return window instanceof TextWindow && window.document == document;
    });
    if (present) {
      present.focus();
      return;
    }
    newTextWindow(parent, document);
  }

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
      createDocumentList();
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
        openInCurrentParent(parent, document);
      else
        newEditorWindow(document);
      ++open_count;
    });
    if (open_count)
      createDocumentList();
  }

  /**
   * @return {!Document}
   */
  function createDocumentList() {
    /**
     * @param {!Document} document
     * @return {!string}
     */
    function stateString(document) {
      return [
        document.modified ? '*' : '-',
        // TODO(yosi) Should be document.readOnly ? '%' : '-'
        '?',
        // TODO(yosi) Should be document.state == 'ready' ? '-' : '!'
        '?',
        document.listWindows().length
      ].join('');
    }

    /** @return {!Document} */
    function getOrNew() {
      /** @const @type {string} */ var NAME = '*document list*';
      var present = Document.find(NAME);
      if (present)
        return present;
      var document = new Document(NAME);
      document.bindKey('Delete', closeSelectedDocuments);
      document.bindKey('Enter', openSelectedDocuments);
      return document;
    }

    var document = getOrNew();
    var range = new Range(document, 0, document.length);
    range.text = '';
    range.text = 'Name\tSize\tState\t\Saved At\tFile\n';
    Document.list.forEach(function(document) {
      range.collapseTo(range.end);
      var fields = [
        document.name,
        document.length,
        stateString(document),
        (new Date()).toLocaleString(), // saved at
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
    var document = createDocumentList();
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
    this.parent.add(table_window);
  }

  Editor.bindKey(Window, 'Ctrl+B', listDocumentCommand);
})();
