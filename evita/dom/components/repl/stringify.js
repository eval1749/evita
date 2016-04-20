// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/** @const @type{{9: string, 10: string, 13:string}} */
const ESCAPE_MAP = {
  0x09: 't',
  0x0A: 'n',
  0x0D: 'r'
};

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

/**
 * @param {!Object} object
 * @return {?function():string}
 */
function findToString(object) {
  for (let runner = object; runner.constructor !== Object;
       runner = Object.getPrototypeOf(runner)) {
    const descriptor = Object.getOwnPropertyDescriptor(runner, 'toString');
    if (descriptor)
      return /** @type {function():string} */ (descriptor.value);
  }
  return null;
}

/**
 * @param {!Object} object
 * @return {string|undefined}
 */
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

// TODO(eval1749): Once v8 provide |TypedArray| class, we can replace this
// |isTypedArray()|.
function isTypedArray(object) {
  return object instanceof Float32Array || object instanceof Float64Array ||
      object instanceof Int16Array || object instanceof Int32Array ||
      object instanceof Int8Array || object instanceof Uint16Array ||
      object instanceof Uint32Array || object instanceof Uint8Array ||
      object instanceof Uint8ClampedArray;
}

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
  for (let runner = object; runner; runner = Object.getPrototypeOf(runner)) {
    if (runner === Object.prototype)
      break;
    const current = /** @type {!Object} */ (runner);
    props = props.concat(
        Object.getOwnPropertyNames(current)
            .map(function(name) {
              const desc = Object.getOwnPropertyDescriptor(
                  /** @type {!Object} */ (current), name);
              desc['name'] = name;
              return desc;
            })
            .filter(function(desc) {
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
// LabelMap
//
class LabelMap extends Visitor {
  constructor() {
    super();
    /** @private @const @type {!Map<*, string>} */
    this.map_ = new Map();
  }

  /**
   * @public
   * @param {*} value
   * @return {string}
   */
  labelOf(value) { return this.map_.get(value) || ''; }

  /**
   * @override
   * @param {*} value
   */
  visitVisited(value) {
    if (this.map_.has(value))
      return;
    this.map_.set(value, (this.map_.size + 1).toString());
  }
}

//////////////////////////////////////////////////////////////////////
//
// StringSink
//
class StringSink extends Visitor {
  /**
   * @param {!LabelMap} labeler
   */
  constructor(labelMap) {
    super();
    /** @const @type {!LabelMap} */
    this.labelMap_ = labelMap;
    /** @type {string} */
    this.result = '';
  }

  /**
   * @private
   * @param {...*} args
   */
  emit(...args) { this.result += args.join(''); }

  /** @override */
  visitAtom(x) { this.emit(x); }

  /** @override */
  visitConstructed(object, id) {
    const ctor = object.constructor;
    const ctorName = ctor && ctor.name !== '' ? ctor.name : '(anonymous)';
    this.emit('#{', ctorName, ' ', id, '}');
  }

  /** @override */
  visitDate(date) { this.emit('#{Date ', date.toString(), '}'); }

  /** @override */
  visitFirstTime(object) {
    const label = this.labelMap_.labelOf(object);
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
    if (fun.name)
      return this.emit(`#{Function ${fun.name}}`);
    this.emit('#{Function}');
  }

  /** @override */
  visitKey(key, index) {
    if (index)
      this.emit(', ');
    this.emit(key, ': ');
  }

  /** @override */
  visitMember(index) {
    if (index === 0)
      return;
    this.emit(', ');
  }

  /** @override */
  visitString(str) {
    this.emit('"');
    for (let index = 0; index < str.length; ++index) {
      /** @const @type {number} */
      const code = str.charCodeAt(index);
      if (code < 0x20 || (code >= 0x7F && code <= 0x9F)) {
        const escape = ESCAPE_MAP[code];
        if (escape) {
          this.emit('\\', escape);
        } else {
          const hex = ('0' + code.toString(16)).substr(-2);
          this.emit('\\0x', hex);
        }
      } else if (code === Unicode.QUOTATION_MARK) {
        this.emit('\\"');
      } else if (code === Unicode.REVERSE_SOLIDUS) {
        this.emit('\\\\');
      } else {
        this.emit(String.fromCharCode(code));
      }
    }
    this.emit('"');
  }

  /** @override */
  visitSymbol(sym) { this.emit(sym.toString()); }

  /** @override */
  visitTypedArray(array) {
    this.emit('#{', array.constructor.name, ' ', array.length, '}');
  }

  /** @override */
  visitVisited(value) {
    const label = this.labelMap_.labelOf(value);
    this.emit('#', label, '#');
  }

  /** @override */
  startContainer(startMark, needSpace) {
    if (needSpace)
      return this.emit(`${startMark} `);
    return this.emit(startMark);
  }

  /** @override */
  endContainer(endMark, index, size) {
    if (index == size)
      return this.emit(endMark);
    return this.emit(`, ...${size}${endMark}`);
  }
}

//////////////////////////////////////////////////////////////////////
//
// Processor
//
class Processor {
  /**
   * @param {number} maxLevel
   * @param {number} maxLength
   * @param {!Visitor} visitor
   */
  constructor(maxLevel, maxLength, visitor) {
    /** @const @type {number} */
    this.maxLevel_ = maxLevel;
    /** @const @type {number} */
    this.maxLength_ = maxLength;
    /** @const @type {!Set<*>} */
    this.processedSet_ = new Set();
    /** @const @type {!Visitor} */
    this.visitor_ = visitor;
  }

  /**
   * @public
   * @param {*} value
   * @param {number} level
   */
  process(value, level) {
    if (value === null)
      return this.visitAtom('null');

    if (value === undefined)
      return this.visitAtom('undefined');

    if (value !== value)
      return this.visitAtom('NaN');

    if (value === Infinity)
      return this.visitAtom('Infinity');

    if (value === -Infinity)
      return this.visitAtom('-Infinity');

    switch (typeof(value)) {
      case 'boolean':
        return this.visitAtom(value.toString());
      case 'function':
        return this.visitor_.visitFunction(value, level);
      case 'number':
        return this.visitAtom(value.toString());
      case 'string':
        return this.visitor_.visitString(value);
      case 'symbol':
        return this.visitor_.visitSymbol(/** @type{!symbol} */ (value));
    }

    const object = /** @type{!Object} */ (value);

    if (this.processedSet_.has(object))
      return this.visitor_.visitVisited(object);
    this.processedSet_.add(object);

    ++level;
    if (level > this.maxLevel_)
      return this.visitAtom('#');

    this.visitor_.visitFirstTime(object);

    if (Array.isArray(object))
      return this.processArray(/** @type{!Array} */ (object), level);

    if (isTypedArray(object))
      return this.visitor_.visitTypedArray(
          /** @type{!ArrayBufferView}*/ (object));

    if (object instanceof Date)
      return this.visitor_.visitDate(/** @type{!Date} */ (object));

    if (object instanceof Map)
      return this.processMap(/** @type {!Map} */ (object), level);

    if (object instanceof Set)
      return this.processSet(/** @type {!Set} */ (object), level);

    return this.processObject(/** @type {!Object} */ (object), level);
  }

  /**
   * @private
   * @param {!Array} array
   * @param {number} level
   */
  processArray(array, level) {
    this.visitor_.startContainer('\u005B', false);
    /** @const @type {number} */
    const length = Math.min(array.length, this.maxLength_);
    /** @type {number} */
    let index = 0;
    while (index < array.length) {
      if (index >= this.maxLength_)
        break;
      this.visitor_.visitMember(index);
      this.process(array[index], level);
      ++index;
    }
    this.visitor_.endContainer('\u005C', index, array.length);
  }

  /**
   * @private
   * @param {!Map} map
   * @param {number} level
   */
  processMap(map, level) {
    this.visitor_.startContainer('#\u007BMap', !!map.size);
    /** @type {number} */
    let index = 0;
    for (const key of map.keys()) {
      if (index >= this.maxLength_)
        break;
      this.visitor_.visitKey(key, index);
      this.process(map.get(key), level + 1);
      ++index;
    }
    this.visitor_.endContainer('\u007D', index, map.size);
  }

  /**
   * @private
   * @param {!Object} object
   * @param {number} level
   */
  processObject(object, level) {
    /** @const @type {?function():string} */
    const toString = findToString(object);
    if (toString)
      return this.visitAtom(toString.call(object));
    /** @const @type {string} */
    const className = computeClassName(object);
    if (className) {
      const id = getObjectIdLikeThing(object);
      if (id !== undefined)
        return this.visitor_.visitConstructed(object, id);
    }
    const props = collectProperties(object);
    if (className)
      this.visitor_.startContainer(`#\u007B${className}`, !!props.length);
    else
      this.visitor_.startContainer(`\u007B`, false);
    /** @type {number} */
    let index = 0;
    for (let prop of props) {
      if (index >= this.maxLength_)
        break;
      this.visitor_.visitKey(prop.name, index);
      this.process(prop.value, level);
      ++index;
    }
    this.visitor_.endContainer('\u007D', index, props.length);
  }

  /**
   * @private
   * @param {!Set} set
   * @param {number} level
   */
  processSet(set, level) {
    this.visitor_.startContainer('#\u007BSet', !!set.size);
    /** @type {number} */
    let index = 0;
    for (const member of set) {
      if (index >= this.maxLength_)
        break;
      this.visitor_.visitMember(index);
      this.process(member, level + 1);
      ++index;
    }
    this.visitor_.endContainer('\u007D', index, set.size);
  }

  /**
   * @private
   * @param {string} string
   */
  visitAtom(string) { this.visitor_.visitAtom(string); }
}

/**
  * Stringify value.
  * @param{*} value
  * @param{number=} opt_maxLevel Default is 10.
  * @param{number=} opt_maxLength Default is 10.
  * @return {string}
  */
function stringify(value, opt_maxLevel = 10, opt_maxLength = 10) {
  /** @const @type {!LabelMap} */
  const labelMap = new LabelMap();
  const labeler = new Processor(opt_maxLevel, opt_maxLength, labelMap);
  labeler.process(value, 0);

  /** @const @type {!StringSink} */
  const stringSink = new StringSink(labelMap);
  const printer = new Processor(opt_maxLevel, opt_maxLength, stringSink);
  printer.process(value, 0);
  return stringSink.result;
}

repl.stringify = stringify;
});
