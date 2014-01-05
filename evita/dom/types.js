// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

var Alter, Direction, Granularity, NewlineMode, Unit;
(function() {
  function makeEnum() {
    var names = Array.prototype.slice.call(arguments, 0);
    return Object.create(Object.prototype, (function() {
      var map = {};
      names.forEach(function(name) {
        map[name.toUpperCase()] = {
          configurable: false,
          enumerable: true,
          value: name,
          writable: false,
        };
      });
      return map;
    })());
  }

  Alter = makeEnum('extend', 'move');
  Direction = makeEnum('backward', 'forward', 'left', 'right');
  NewlineMode = makeEnum('cr', 'crlf', 'lf', 'unknown');
  Unit = makeEnum('char', 'document', 'line', 'page', 'paragraph',
                  'screen', 'sentence', 'window', 'word');
  Granularity = Unit;
})();

function EnumError(fields, message) {
  TypeError.call(this, message);
  this.fields = fields;
  // TODO(yosi) This is work around of V8 bug. We should not have
  // |EnumError.message| here.
  this.message = message;
}

EnumError.prototype = Object.create(TypeError.prototype, {
  fields: {
    configurable: true,
    enumerable: true,
    value: [],
    writable: true,
  }, // fields
  toString: {
    value: function() {
      return this.message + ' accepts only ' + this.fields + '.';
    },
    configurable: true,
    enumerable: true,
  }, // toString
});
