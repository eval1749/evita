// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  /**
   * @param {!Object} object
   * @return {string}
   */
  function computeClassName(object) {
    if (Symbol.toStringTag in object)
      return object[Symbol.toStringTag];
    const constructor = object.constructor;
    if (!constructor || constructor === Object)
      return '';
    if ('name' in constructor)
      return constructor.name;
    return '';
  }

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
    visitMember(index) {}

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

      /**
       * @param {string} startMark
       * @param {boolean} needSpace
       */
    startContainer(startMark, needSpace) {}

      /**
       * @param {string} endMark
       * @param {number} index
       * @param {number} size
       */
    endContainer(endMark, index, size) {}
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
    let numLabels = 0;

    /**
     * @param {!Object} object
     * @return {!Array}
     */
    function collectProperties(object) {
      const override = object['stringifyProperties'];
      if (object !== object.constructor.prototype &&
          typeof(override) === 'function') {
        return override.call(object);
      }
      let props = [];
      const removeFunction = object.constructor !== Object;
      for (let runner = object; runner;
           runner = Object.getPrototypeOf(runner)) {
        if (runner === Object.prototype)
          break;
        const current = /** @type {!Object} */(runner);
        props = props.concat(
            Object.getOwnPropertyNames(current).map(function(name) {
              const desc = Object.getOwnPropertyDescriptor(
                /** @type {!Object} */(current), name);
              desc['name'] = name;
              return desc;
            }).filter(function(desc) {
              const name = desc['name'];
              if (name.charCodeAt(name.length - 1) === Unicode.LOW_LINE)
                return false;
              const value = desc['value'];
              if (removeFunction && typeof(value) === 'function')
                return false;
              return value !== undefined;
            }));
      }
      return props.sort((a, b) => a.name.localeCompare(b.name));
    }

    /** @param {!Object} object @return {string|undefined} */
    function getObjectIdLikeThing(object) {
      try {
        const key = ['id', 'name'].find(function(key) {
          const value = object[key];
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
     * @param {!Array} array
     * @param {number} level
     */
    function visitArray(visitor, array, level) {
      visitor.startContainer('[', false);
      const length = Math.min(array.length, maxLength);
      let index = 0;
      while (index < array.length) {
        if (index >= maxLength)
          break;
        visitor.visitMember(index);
        visit(array[index], level, visitor);
        ++index;
      }
      visitor.endContainer(']', index, array.length);
    }

    /**
     * @param {!Visitor} visitor
     * @param {!Map} map
     * @param {number} level
     */
    function visitMap(visitor, map, level) {
      visitor.startContainer('#\u007BMap', !!map.size);
      let index = 0;
      for (const key of map.keys()) {
        if (index >= maxLength)
          break;
        visitor.visitKey(key, index);
        visit(map.get(key), level + 1, visitor);
        ++index;
      }
      visitor.endContainer('\u007D', index, map.size);
    }


    /**
     * @param {!Visitor} visitor
     * @param {!Object} object
     * @param {number} level
     */
    function visitObject(visitor, object, level) {
      const className = computeClassName(object);
      if (className) {
        const id = getObjectIdLikeThing(object);
        if (id !== undefined)
          return visitor.visitConstructed(object, id);
      }
      const props = collectProperties(object);
      if (className)
        visitor.startContainer(`#\u007B${className}`, !!props.length);
      else
        visitor.startContainer(`\u007B`, false);
      let index = 0;
      for (let prop of props) {
        if (index >= maxLength)
          break;
        visitor.visitKey(prop.name, index);
        visit(prop.value, level, visitor);
        ++index;
      }
      visitor.endContainer('\u007D', index, props.length);
    }

    /**
     * @param {!Visitor} visitor
     * @param {!Set} set
     * @param {number} level
     */
    function visitSet(visitor, set, level) {
      visitor.startContainer('#\u007BSet', !!set.size);
      let index = 0;
      for (const member of set) {
        if (index >= maxLength)
          break;
        visitor.visitMember(index);
        visit(member, level + 1, visitor);
        ++index;
      }
      visitor.endContainer('\u007D', index, set.size);
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

      const object = /** @type{!Object} */(value);

      if (visitedMap.has(object))
        return visitor.visitVisited(object);
      visitedMap.set(object, 0);

      ++level;
      if (level > maxLevel)
        return visitor.visitAtom('#');

      visitor.visitFirstTime(object);
      visitedMap.set(object, 0);

      if (Array.isArray(object))
        return visitArray(visitor, /** @type{!Array} */(object), level);

      if (isTypedArray(object))
        return visitor.visitTypedArray(/** @type{!ArrayBufferView}*/(object));

      if (object instanceof Date)
        return visitor.visitDate(/** @type{!Date} */(object));

      if (object instanceof Map)
        return visitMap(visitor, /** @type {!Map} */(object), level);

      if (object instanceof Set)
        return visitSet(visitor, /** @type {!Set} */(object), level);

      return visitObject(visitor, /** @type {!Object} */(object), level);
    }

    /** @const @type{{9: string, 10: string, 13:string}} */
    const ESCAPE_MAP = {
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
        const ctor = object.constructor;
        const ctorName = ctor && ctor.name !== '' ? ctor.name : '(anonymous)';
        this.emit('#{', ctorName, ' ' , id, '}');
      }

      visitDate(date) {
        this.emit('#{Date ', date.toString(), '}');
      }

      visitFirstTime(object) {
        const label = this.labeler_.labelOf(object);
        if (!label)
          return;
        this.emit('#', label, '=');
      }

      /** @override */
      visitFunction(fun, level) {
        if (!level) {
          this.emit('(', fun.toString().replace(/\s+/g, ' '), ')');
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
        this.emit(key, ': ');
      }

      visitMember(index) {
        if (index === 0)
          return;
        this.emit(', ');
      }

      visitString(str) {
        this.emit('"');
        for (let index = 0; index < str.length; ++index) {
          const code = str.charCodeAt(index);
          if (code < 0x20 || (code >= 0x7F && code <= 0x9F)) {
            const escape = ESCAPE_MAP[code];
            if (escape) {
              this.emit('\\', escape);
            } else {
              const hex = ('0' + code.toString(16)).substr(-2);
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
        const label = this.labeler_.labelOf(value);
        this.emit('#', label, '#');
      }

      startContainer(startMark, needSpace) {
        if (needSpace)
          return this.emit(`${startMark} `);
        return this.emit(startMark);
      }

      endContainer(endMark, index, size) {
        if (index == size)
          return this.emit(endMark);
        return this.emit(`, ...${size}${endMark}`);
      }
    }

    const labeler = new Labeler();
    visit(value, 0, labeler);
    visitedMap.clear();
    const printer = new Printer(labeler);
    visit(value, 0, printer);
    return printer.result;
  }

  $export({stringify});
});
