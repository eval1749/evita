// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  /**
   * @this {TableWindow}
   */
  function closeSelectedDocuments() {
    var documents = Document.list;
    var keys = documents.map(function(document) {
      return document.name;
    });
    var needUpdate = false;
    this.selection.getRowStates(keys).forEach(function(state, index) {
      if (!(state & TableViewRowState.SELECTED))
        return;
      documents[index].close();
      // TODO(yosi) We should handle Document event rather than using
      // |needUpdate| variable. Because |close()| is asynchronus operation
      // and document may not be closed yet or cancled.
      needUpdate = true;
    });
    if (needUpdate)
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
      /** @const @type {string} */ var NAME = '*buffer list*';
      var present = Document.find(NAME);
      if (present)
        return present;
      var document = new Document(NAME);
      document.bindKey('Delete', closeSelectedDocuments);
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
