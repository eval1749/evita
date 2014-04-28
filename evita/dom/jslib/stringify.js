// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Editor.stringify = (function() {
  /** @interface */
  function Visitor() {}

    /** @param {number} index */
  Visitor.prototype.visitArrayElement = function(index) {};

    /** @param {*} atom */
  Visitor.prototype.visitAtom = function(atom) {};

  /**
   * @param {!Object} object
   * @param {*} id
   */
  Visitor.prototype.visitConstructed = function(object, id) {};

    /** @param {!Date} date*/
  Visitor.prototype.visitDate = function(date) {};

    /** @param {!Object} object */
  Visitor.prototype.visitFirstTime = function(object) {};

    /** @param {!Function} fun @param {number} level */
  Visitor.prototype.visitFunction = function(fun, level) {};

    /** @param {*} key @param {number} index */
  Visitor.prototype.visitKey = function(key, index) {};

  /** @param {string} string */
  Visitor.prototype.visitString = function(string) {};

  /** @param {!Symbol} symbol */
  Visitor.prototype.visitSymbol = function(symbol) {};

  /** @param {!TypedArray} array */
  Visitor.prototype.visitTypedArray = function(array) {};

    /** @param {!Object} object */
  Visitor.prototype.visitVisited = function(object) {};

    /** @param {!Array} array */
  Visitor.prototype.startArray = function(array) {};

    /** @param {!Array} array @param {boolean} limited */
  Visitor.prototype.endArray = function(array, limited) {};

    /** @param {!Object} props @param {string} ctor_name */
  Visitor.prototype.startObject = function(ctor_name, props) {};

    /** @param {boolean} limited */
  Visitor.prototype.endObject = function(limited) {};

  /**
   * @constructor
   * @implements {Visitor}
   */
  function Labeler() {
    var label_map = new Map();
    var doNothing = function() {};
    this.labelOf = function(value) {
      return label_map.get(value);
    };
    this.visitArrayElement = doNothing;
    this.visitAtom = doNothing;
    this.visitConstructed = doNothing;
    this.visitDate= doNothing;
    this.visitFirstTime = doNothing;
    this.visitFunction = doNothing;
    this.visitKey = doNothing;
    this.visitString = doNothing;
    this.visitSymbol = doNothing;
    this.visitTypedArray = doNothing;
    this.visitVisited = function(value) {
      if (label_map.has(value))
        return;
      label_map.set(value, (label_map.size + 1).toString());
    };
    this.startArray = doNothing;
    this.endArray = doNothing;
    this.startObject = doNothing;
    this.endObject = doNothing;
  }

  /**
    * Stringify value.
    * @param{*} value
    * @param{number=} MAX_LEVEL Default is 10.
    * @param{number=} MAX_LENGTH Default is 10.
    * @return {string}
    */
  function stringify(value, MAX_LEVEL, MAX_LENGTH) {
    MAX_LEVEL = arguments.length >= 1 ? MAX_LEVEL : 10;
    MAX_LENGTH = arguments.length >= 2 ? MAX_LENGTH : 10;
    var visited_map = new Map();
    var num_of_labels = 0;

    /**
     * @param {!Object} object
     * @return {!Array}
     */
    function collectProperties(object) {
      var override = object['stringifyProperties'];
      if (typeof(override) == 'function')
        return override.call(object);
      var props = [];
      var remove_function = object.constructor != Object;
      for (var runner = object; runner;
           runner = Object.getPrototypeOf(runner)) {
        if (runner === Object.prototype)
          break;
        var current = /** @type{!Object} */(runner);
        props = props.concat(
            Object.getOwnPropertyNames(current).map(function(name) {
              var desc = Object.getOwnPropertyDescriptor(current, name);
              desc['name'] = name;
              return desc;
            }).filter(function(desc) {
              var value = desc['value'];
              if (remove_function && typeof(value) == 'function')
                return false;
              return value !== undefined;
            }))
      }
      return props.sort(function(a, b) {
        return a.name.localeCompare(b.name);
      });
    }

    /** @param {!Object} object @return {string|undefined} */
    function getObjectIdLikeThing(object) {
      try {
        var key = ['id', 'name'].find(function(key) {
          var value = object[key];
          return value != undefined && value.toString().length;
        });
        return key ? object[key] : undefined;
      } catch (e) {
        // We can't acccess |Window.prototype.id|, because it has no C++
        // object associated it.
        return undefined;
      }
    }

    /**
     * @param {*} value
     * @param {number} level
     * @param {!Visitor} visitor
     */
    function visit(value, level, visitor) {
      if (value === null)
        return visitor.visitAtom('null');

      if (value === undefined)
        return visitor.visitAtom('undefined');

      if (value !== value)
        return visitor.visitAtom('NaN');

      if (value === Infinity)
        return visitor.visitAtom('Infinity');

      if (value === -Infinity)
        return visitor.visitAtom('-Infinity');

      switch (typeof(value)) {
        case 'boolean':
          return visitor.visitAtom(value.toString());
        case 'function':
          return visitor.visitFunction(value, level);
        case 'number':
          return visitor.visitAtom(value.toString());
        case 'string':
          return visitor.visitString(value);
        case 'symbol':
          return visitor.visitSymbol(/** @type{!Symbol} */(value));
      }

      var object = /** @type{!Object} */(value);

      if (visited_map.has(object))
        return visitor.visitVisited(object);
      visited_map.set(object, 0);

      ++level;
      if (level > MAX_LEVEL)
        return visitor.visitAtom('#');

      visitor.visitFirstTime(object);
      visited_map.set(object, 0);

      if (Array.isArray(object)) {
        var array = /** @type{!Array} */(object);
        visitor.startArray(array);
        var length = Math.min(array.length, MAX_LENGTH);
        for (var index = 0; index < length; ++index) {
          visitor.visitArrayElement(index);
          visit(array[index], level, visitor);
        }
        return visitor.endArray(array, array.length >= MAX_LENGTH);
      }

      // TODO(yosi) Once v8 provide |TypedArray| class, we can replace this
      // |isTypedArray()|.
      function isTypedArray(object) {
        return object instanceof Float32Array ||
               object instanceof Float64Array ||
               object instanceof Int16Array ||
               object instanceof Int32Array ||
               object instanceof Int8Array ||
               object instanceof Uint16Array ||
               object instanceof Uint32Array ||
               object instanceof Uint8Array ||
               object instanceof Uint8ClampedArray;
      }

      if (isTypedArray(object)) {
        return visitor.visitTypedArray(/** @type{!TypedArray}*/(object));
      }

      if (object instanceof Date)
        return visitor.visitDate(/** @type{!Date} */(object));

      var real_ctor_name = object.constructor.name;
      var ctor_name = real_ctor_name == 'Object' ? '' : real_ctor_name;
      if (ctor_name) {
        var id = getObjectIdLikeThing(object);
        if (id !== undefined)
          return visitor.visitConstructed(object, id);
      }
      var props = collectProperties(object);
      visitor.startObject(ctor_name, props);
      var count = 0;
      props.forEach(function(prop) {
        if (count > MAX_LENGTH)
          return;
        visitor.visitKey(prop.name, count);
        visit(prop.value, level, visitor);
        ++count;
      });
      visitor.endObject(count > props.length);
    }

    /** @const @type{{9: string, 10: string, 13:string}} */
    var ESCAPE_MAP = {
      0x09: 't',
      0x0A: 'n',
      0x0D: 'r'
    };

    /**
     * @constructor
     * @implements {Visitor}
     * @param {!Labeler} labeler
     */
    function Printer(labeler) {
      this.result = '';
      /** @param {...} var_args */
      this.emit = function(var_args) {
        this.result += Array.prototype.slice.call(arguments, 0).join('');
      };
      this.visitAtom = function(x) {
        this.emit(x);
      };
      this.visitConstructed = function(object, id) {
        var ctor = object.constructor;
        var ctor_name = ctor && ctor.name != '' ? ctor.name : '(anonymous)';
        this.emit('#{', ctor_name, ' ' , id, '}');
      };
      this.visitDate = function(date) {
        this.emit('#{Date ', date.toString(), '}');
      };
      this.visitFirstTime = function(object) {
        var label = labeler.labelOf(object);
        if (label)
          this.emit('#', label, '=');
      };
      this.visitFunction = function(fun, level) {
        if (!level) {
          this.emit('(', fun.toString(), ')');
          return;
        }
        if (fun.name)
          this.emit('#{Function ' + fun.name + '}');
        else
          this.emit('#{Function}');
      };
      this.visitKey = function(key, index) {
        if (index)
          this.emit(', ');
        this.emit(key.toString(), ': ');
      };
      this.visitString = function(str) {
        this.emit('"');
        for (var index = 0; index < str.length; ++index) {
          var code = str.charCodeAt(index);
          if (code < 0x20 || (code >= 0x7F && code <= 0x9F)) {
            var escape = ESCAPE_MAP[code];
            if (escape) {
              this.emit('\\', escape);
            } else {
              var hex = ('0' + code.toString(16)).substr(code >= 0x10 ? 1 : 0);
              this.emit('\\0x', hex);
            }
          } else if (code == 0x22) {
            this.emit('\\"');
          } else if (code == 0x5C) {
            this.emit('\\\\');
          } else {
            this.emit(String.fromCharCode(code));
          }
        }
        this.emit('"');
      };
      this.visitSymbol = function(sym) {
        this.emit(sym.toString());
      };
      this.visitTypedArray = function(array) {
        this.emit('#{', array.constructor.name, ' ', array.length, '}');
      };
      this.visitVisited = function(value) {
        var label = labeler.labelOf(value);
        this.emit('#', label, '#');
      };
      this.startArray = function() {
        this.emit('[');
      };
      this.endArray = function(array, limited) {
        this.emit(limited ? ', ...' + array.length + ']' : ']');
      };
      this.visitArrayElement = function(index) {
        if (index)
          this.emit(', ');
      };
      this.startObject = function(ctor_name, props) {
        if (ctor_name)
          this.emit('#{', ctor_name, props.length ? ' ' : '');
        else
          this.emit('{');
      };
      this.endObject = function(limited) {
        this.emit(limited ? ', ...}' : '}');
      };
    }

    var labeler = new Labeler();
    visit(value, 0, labeler);
    visited_map.clear();
    var printer = new Printer(labeler);
    visit(value, 0, printer);
    return printer.result;
  }

  return stringify;
})();
