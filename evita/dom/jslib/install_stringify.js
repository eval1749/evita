// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// Install printers.
Error.prototype.stringifyProperties = function() {
  return [{name: 'message', value: this.message},
          {name: 'stack', value: this.stack}];
};

Event.prototype.stringifyProperties = function() {
  return [{name: 'type', value: this.type},
          {name: 'target', value: this.target}];
};

Point.prototype.stringifyProperties = function() {
  return [{name: 'x', value: this.x}, {name: 'y', value: this.y}];
};

Range.prototype.stringifyProperties = function() {
  return [{name: 'document', value: this.document},
          {name: 'start', value: this.start},
          {name: 'end', value: this.end}];
};

TextSelection.prototype.stringifyProperties = function() {
  return [{name: 'document', value: this.document},
          {name: 'start', value: this.range.start},
          {name: 'end', value: this.range.end}];
};
