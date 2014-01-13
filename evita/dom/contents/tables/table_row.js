// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

'use strict';

(function() {
  /**
   * @this {TableRow}
   * @param {Object...}
   * @return {TableRow}
   */
  TableRow.prototype.addCell = function() {
    var row = this;
    Array.prototype.slice.call(arguments, 0).forEach(function(value) {
      var cell = value instanceof TableCell ? cell :
          new TableCell(value.toString());
      row.addCell_(cell);
    });
    return row;
  };

  /**
   * @this {TableRow}
   * @return {TableRow}
   */
  TableRow.prototype.removeAllCells = function() {
    var row = this;
    while (row.length) {
      row.removeCell(0);
    }
    return row;
  };
})();
