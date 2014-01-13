// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

'use strict';

(function() {
  /**
   * @this {TableContent}
   * @param {Object...}
   * @return {TableRow}
   */
  TableContent.prototype.addCell = function() {
    var row = new TableRow();
    row.addCell.apply(row, arguments);
    this.addRow(row);
    return row;
  };

  /**
   * @this {TableContent}
   * @param {Object...}
   * @return {TableCell}
   */
  TableContent.prototype.cell = function(column_index, row_index) {
    return this.row(row_index).cell(column_index);
  };

  /**
   * @this {TableContent}
   * @return {TableContent}
   */
  TableContent.prototype.removeAllRows = function() {
    var table = this;
    while (table.length) {
      table.removeRow(0);
    }
    return table;
  };

  /**
   * @this {TableContent}
   * @return {TableContent}
   */
  TableContent.prototype.reset = function() {
    var table = this;
    table.headerRow.removeAllCells();
    table.removeAllRows();
  };
})();
