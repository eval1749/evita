// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  const kNativeCode = ' { [native code] }';
  const kMaxLength = 40;

  /**
   * @param {!Function} thing
   */
  function describeFunction(thing) {
    console.log('A function', thing.name);
    describeObject(thing);
  }

  /**
   * @param {!number} thing
   */
  function describeNumber(thing) {
    console.log('A number', thing);
    printAsTable([
        {key: 'Binary', value: '0b' + thing.toString(2)},
        {key: 'Octal', value: '0o' + thing.toString(8)},
        {key: 'Decimal', value: thing.toString(10)},
        {key: 'Hexadecimal', value: '0x' + thing.toString(16)},
        {key: 'Base 36', value: thing.toString(36)}]);
  }

  /**
   * @param {!Object} thing
   */
  function describeObject(thing) {
    let isFirst = true;
    for (let runner = thing; runner; runner = Object.getPrototypeOf(runner)) {
      if (isFirst) {
        const keys = Object.keys(thing);
        console.log('An object', thing, 'which has', keys.length, 'keys');
        isFirst = false;
      } else {
        console.log('Prototype', runner.constructor.name);
      }

      const keys = Object.getOwnPropertyNames(runner).concat(
          // TODO(eval1749): We should use '.'-syntax once closure compiler
          // having correct declaration.
          Object['getOwnPropertySymbols'](runner));

      printAsTable(keys.map(key => {
        const descriptor = Object.getOwnPropertyDescriptor(
            /** @type {!Object} */(runner), key);
        const name = key.toString();
        const isGetter = 'get' in descriptor;
        const isSetter = 'get' in descriptor;
        if (isGetter && isSetter)
          return {key: name, value: 'getter and setter'};
        if (isGetter)
          return {key: name, value: 'getter'};
        if (isSetter)
          return {key: name, value: 'setter'};
        return {key: name, value: descriptor.value};
      }).sort((a, b) => a.key.localeCompare(b.key)));
    }
  }

  /**
   * @param {!string} thing
   */
  function describeString(thing) {
    console.log('A string', formatValue(thing));
    printAsTable([
      {key: 'HasNonAscii', value: hasNonAscii(thing)},
      {key: 'Length', value: thing.length},
      {key: 'LowerCase', value: thing.toLowerCase()},
      {key: 'UpperCase', value: thing.toUpperCase()}]);
  }

  /**
   * @param {!symbol} thing
   */
  function describeSymbol(thing) {
    console.log('A symbol', thing);
    printAsTable([
      {key: 'Is shared', value: Symbol.keyFor(thing) ? 'yes' : 'no'}]);
  }

  /**
   * @param {!Function} value
   * @return {string}
   */
  function formatFunction(value) {
    const string = value.toString().replace(/\s+/g, ' ');
    if (string.endsWith(kNativeCode))
      return string.substr(0, string.length - kNativeCode.length);
    if (string.length < kMaxLength)
      return string;
    return string.substring(0, kMaxLength) + '...';
  }

  /**
   * @param {string} value
   * @return {string}
   */
  function formatString(value) {
    const formatted = repl.stringify(value);
    if (formatted.length <= kMaxLength)
      return formatted;
    return formatted.substr(0, 40) + `...${value.length}"`;
  }

  /**
   * @param {*} value
   * @return {*}
   */
  function formatValue(value) {
    switch (typeof(value)) {
      case 'function':
        return formatFunction(value);
      case 'string':
        return formatString(value);
    }
    return value;
  }

  /**
   * @param {string} string
   * @return {boolean}
   */
  function hasNonAscii(string) {
    for (let i = 0; i < string.length; ++i) {
      const charCode = string.charCodeAt(i);
      if (charCode > 0xFF)
        return true;
    }
    return false;
  }

  /**
   * @param {!Array.<{key: string, value: *}>} rows
   */
  function printAsTable(rows) {
    const maxKeyLen = rows.reduce((previous, {key}) => {
      return Math.max(previous, key.length);
    }, 0);
    for (let {key, value} of rows) {
      console.log(' ', key + ':' + ' '.repeat(maxKeyLen - key.length),
                  formatValue(value));
    }
  }

  /**
   * @param {*} object
   * @return {undefined}
   */
  function describe(object) {
    switch (typeof(object)) {
      case 'function':
        describeFunction(/** @type {!Function} */(object));
        break;
      case 'number':
        describeNumber(/** @type {number} */(object));
        break;
      case 'object':
        describeObject(/** @type {!Object} */(object));
        break;
      case 'string':
        describeString(/** @type {string} */(object));
        break;
      case 'symbol':
        describeSymbol(/** @type {symbol} */(object));
        break;
      case 'undefined':
        console.log('An undefined value');
        break;
      default:
        console.log('Unknown type', typeof(object), object);
        break;
    }
    return undefined;
  }

  $export({describe});
});

global.describe = repl.describe;
