// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  // TODO(eval1749): Once v8 provide |TypedArray| class, we can replace this
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

  class Visitor {
    /** @param {number} index */
    visitArrayElement(index) {}

    /** @param {*} atom */
    visitAtom(atom) {}

    /**
     * @param {!Object} object
     * @param {*} id
     */
    visitConstructed(object, id) {}

    /** @param {!Date} date*/
    visitDate(date) {}

      /** @param {!Object} object */
    visitFirstTime(object) {}

      /** @param {!Function} fun @param {number} level */
    visitFunction(fun, level) {}

      /** @param {*} key @param {number} index */
    visitKey(key, index) {}

    /** @param {number} index */
    visitSetMember(index) {}

    /** @param {string} string */
    visitString(string) {}

    /** @param {!symbol} symbol */
    visitSymbol(symbol) {}

    /** @param {!ArrayBufferView} array */
    visitTypedArray(array) {}

      /** @param {!Object} object */
    visitVisited(object) {}

      /** @param {!Array} array */
    startArray(array) {}

      /** @param {!Array} array @param {boolean} limited */
    endArray(array, limited) {}

      /** @param {!Set} set */
    startSet(set) {}

      /** @param {!Set} set @param {boolean} limited */
    endSet(set, limited) {}

      /** @param {!Object} props @param {string} ctorName */
    startObject(ctorName, props) {}

      /** @param {boolean} limited */
    endObject(limited) {}
  }

  ////////////////////////////////////////////////////////////
  //
  // Labeler
  //
  class Labeler extends Visitor {
    constructor() {
      super();
      /** @private @const */
      this.labelMap_ = new Map();
    }

    labelOf(value) {
      return this.labelMap_.get(value);
    }

    visitVisited(value) {
      if (this.labelMap_.has(value))
        return;
      this.labelMap_.set(value, (this.labelMap_.size + 1).toString());
    }
  }

  /**
    * Stringify value.
    * @param{*} value
    * @param{number=} opt_maxLevel Default is 10.
    * @param{number=} opt_maxLength Default is 10.
    * @return {string}
    */
  function stringify(value, opt_maxLevel = 10, opt_maxLength = 10) {
    const maxLevel = /** @type {number} */(opt_maxLevel);
    const maxLength = /** @type {number} */(opt_maxLength);
    const visitedMap = new Map();
    let numOf_labels = 0;

    /**
     * @param {!Object} object
     * @return {!Array}
     */
    function collectProperties(object) {
      let override = object['stringifyProperties'];
      if (object !== object.constructor.prototype &&
          typeof(override) === 'function') {
        return override.call(object);
      }
      let props = [];
      let removeFunction = object.constructor !== Object;
      for (let runner = object; runner;
           runner = Object.getPrototypeOf(runner)) {
        if (runner === Object.prototype)
          break;
        let current = /** @type {!Object} */(runner);
        props = props.concat(
            Object.getOwnPropertyNames(current).map(function(name) {
              let desc = Object.getOwnPropertyDescriptor(
                /** @type {!Object} */(current), name);
              desc['name'] = name;
              return desc;
            }).filter(function(desc) {
              let name = desc['name'];
              if (name.charCodeAt(name.length - 1) === Unicode.LOW_LINE)
                return false;
              let value = desc['value'];
              if (removeFunction && typeof(value) === 'function')
                return false;
              return value !== undefined;
            }));
      }
      return props.sort(function(a, b) {
        return a.name.localeCompare(b.name);
      });
    }

    /** @param {!Object} object @return {string|undefined} */
    function getObjectIdLikeThing(object) {
      try {
        let key = ['id', 'name'].find(function(key) {
          let value = object[key];
          return value !== undefined && value.toString().length;
        });
        return key ? object[key] : undefined;
      } catch (e) {
        // We can't access |Window.prototype.id|, because it has no C++
        // object associated it.
        return undefined;
      }
    }

    /**
     * @param {!Visitor} visitor
     * @param {!Set} set
     * @param {number} level
     */
    function visitSet(visitor, set, level) {
      visitor.startSet(set);
      let index = 0;
      for (const member of set) {
        if (index >= maxLength)
          break;
        visitor.visitSetMember(index);
        visit(member, level + 1, visitor);
        ++index;
      }
      visitor.endSet(set, set.size > index);
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
          return visitor.visitSymbol(/** @type{!symbol} */(value));
      }

      let object = /** @type{!Object} */(value);

      if (visitedMap.has(object))
        return visitor.visitVisited(object);
      visitedMap.set(object, 0);

      ++level;
      if (level > maxLevel)
        return visitor.visitAtom('#');

      visitor.visitFirstTime(object);
      visitedMap.set(object, 0);

      if (Array.isArray(object)) {
        let array = /** @type{!Array} */(object);
        visitor.startArray(array);
        let length = Math.min(array.length, maxLength);
        for (var index = 0; index < length; ++index) {
          visitor.visitArrayElement(index);
          visit(array[index], level, visitor);
        }
        return visitor.endArray(array, array.length >= maxLength);
      }

      if (isTypedArray(object))
        return visitor.visitTypedArray(/** @type{!ArrayBufferView}*/(object));

      if (object instanceof Date)
        return visitor.visitDate(/** @type{!Date} */(object));

      if (object instanceof Set)
        return visitSet(visitor, /** @type {!Set} */(object), level);

      let realCtorName = object.constructor ? object.constructor.name : '';
      let ctorName = realCtorName === 'Object' ? '' : realCtorName;
      if (ctorName) {
        let id = getObjectIdLikeThing(object);
        if (id !== undefined)
          return visitor.visitConstructed(object, id);
      }
      let props = collectProperties(object);
      visitor.startObject(ctorName, props);
      let count = 0;
      for (let prop of props) {
        if (count > maxLength)
          break;
        visitor.visitKey(prop.name, count);
        visit(prop.value, level, visitor);
        ++count;
      }
      visitor.endObject(count > props.length);
    }

    /** @const @type{{9: string, 10: string, 13:string}} */
    let ESCAPE_MAP = {
      0x09: 't',
      0x0A: 'n',
      0x0D: 'r'
    };

    //////////////////////////////////////////////////////////////////////
    //
    // Printer
    //
    class Printer extends Visitor {
      constructor(labeler) {
        super();
        this.labeler_ = labeler;
        this.result = '';
      }

      /** @param {...} varArgs */
      emit(varArgs) {
        this.result += Array.prototype.slice.call(arguments, 0).join('');
      }

      visitAtom(x) {
        this.emit(x);
      }

      visitConstructed(object, id) {
        let ctor = object.constructor;
        let ctorName = ctor && ctor.name !== '' ? ctor.name : '(anonymous)';
        this.emit('#{', ctorName, ' ' , id, '}');
      }

      visitDate(date) {
        this.emit('#{Date ', date.toString(), '}');
      }

      visitFirstTime(object) {
        let label = this.labeler_.labelOf(object);
        if (!label)
          return;
        this.emit('#', label, '=');
      }

      visitFunction(fun, level) {
        if (!level) {
          this.emit('(', fun.toString(), ')');
          return;
        }
        if (fun.name) {
          this.emit('#{Function ' + fun.name + '}');
          return;
        }
        this.emit('#{Function}');
      }

      visitKey(key, index) {
        if (index)
          this.emit(', ');
        this.emit(key.toString(), ': ');
      }

      visitString(str) {
        this.emit('"');
        for (var index = 0; index < str.length; ++index) {
          let code = str.charCodeAt(index);
          if (code < 0x20 || (code >= 0x7F && code <= 0x9F)) {
            let escape = ESCAPE_MAP[code];
            if (escape) {
              this.emit('\\', escape);
            } else {
              let hex = ('0' + code.toString(16)).substr(code >= 0x10 ? 1 : 0);
              this.emit('\\0x', hex);
            }
          } else if (code === 0x22) {
            this.emit('\\"');
          } else if (code === 0x5C) {
            this.emit('\\\\');
          } else {
            this.emit(String.fromCharCode(code));
          }
        }
        this.emit('"');
      }

      visitSymbol(sym) {
        this.emit(sym.toString());
      }

      visitTypedArray(array) {
        this.emit('#{', array.constructor.name, ' ', array.length, '}');
      }

      visitVisited(value) {
        let label = this.labeler_.labelOf(value);
        this.emit('#', label, '#');
      }

      startArray() {
        this.emit('\u005b');
      }

      visitArrayElement(index) {
        if (!index)
          return;
        this.emit(', ');
      }

      endArray(array, limited) {
        if (limited)
          this.emit(', ...' + array.length);
        this.emit('\u005d');
      }

      startObject(ctorName, props) {
        if (ctorName) {
          this.emit('#{', ctorName, props.length ? ' ' : '');
          return;
        }
        this.emit('\u007b');
      }

      endObject(limited) {
        this.emit(limited ? ', ...}' : '}');
      }

      startSet() {
        this.emit('#\u007bSet');
      }

      visitSetMember(index) {
        this.emit(index ? ', ' : ' ');
      }

      endSet(set, limited) {
        if (limited)
          this.emit(', ...' + set.size);
        this.emit('\u007d');
      }
    }

    let labeler = new Labeler();
    visit(value, 0, labeler);
    visitedMap.clear();
    let printer = new Printer(labeler);
    visit(value, 0, printer);
    return printer.result;
  }

  $export({stringify});
});
