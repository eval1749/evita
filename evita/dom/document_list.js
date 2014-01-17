// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
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
    var document = Document.getOrNew('*buffer list*');
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

  Editor.bindKey('Ctrl+B', listDocumentCommand);
})();
