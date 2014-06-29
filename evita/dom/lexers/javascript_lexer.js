// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.JavaScriptLexer = (function(options) {
  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   * @param {!Lexer=} opt_parentLexer
   */
  function JavaScriptLexer(document, opt_parentLexer) {
    this.parentLexer_ = arguments.length >= 2 ?
        /** @type {!Lexer} */(opt_parentLexer) : null;
    ClikeLexer.call(this, document, options);
  }

  JavaScriptLexer.keywords = options.keywords;
  JavaScriptLexer.prototype = Object.create(ClikeLexer.prototype, {
    constructor: {value: JavaScriptLexer},
  });

  return JavaScriptLexer;
})({
  useDot: true,
  characters: ClikeLexer.newCharacters(),
  keywords: (function() {
    var map = Lexer.createKeywords([
      'break',
      'case', 'catch', 'class', 'const', 'continue',
      'debugger',
      'default', 'delete', 'do',
      'else', 'export', 'extends',
      'finally', 'for', 'function',
      'if', 'import', 'in', 'instanceof',
      'new',
      'return',
      'super',
      'switch',
      'this', 'throw', 'try', 'typeof',
      'var', 'void',
      'while', 'with',
      'yield',

      // Reserved label
      'default:',

      // Literals
      'false', 'null', 'true',

      // Future reserved word in ES6
      'enum',
      'implements',
      'interface',
      'package',
      'private',
      'protected',
      'public',
      'static',

      // Standard build-int objects
      // Value properties
      'Infinity',
      'NaN',
      'undefined',

      // Function properties
      'eval',
      'uneval',
      'isFinite',
      'isNaN',
      'parseFloat',
      'parseInt',
      'decodeURI',
      'decodeURIComponent',
      'encodeURI',
      'encodeURIComponent',
      'escape',
      'unescape',

      // Fundamental objects
      'Object',
        'Object.create', 'Object.defineProperties', 'Object.defineProperty',
        'Object.freeze', 'Object.getOwnPropertyDescriptor',
        'Object.getOwnPropertyNames', 'Object.getPrototypeOf', 'Object.is',
        'Object.isExtensible', 'Object.isFrozen', 'Object.isSealed',
        'Object.keys', 'Object.preventExtensions', 'Object.seal',
        'Object.setPrototypeOf',
      'Function',
      'Boolean',
      'Symbol',
      'Error',
      'EvalError',
      'InternalError',
      'RangeError',
      'ReferenceError',
      'StopIteration',
      'SyntaxError',
      'TypeError',
      'URIError',

      // Numbers and dates
      'Number',
        'Number.EPSILON', 'Number.MAX_VALUE', 'Number.MIN_VALUE',
        'Number.NEGATIVE_INFINITY', 'Number.NaN', 'Number.POSITIVE_INFINITY',
        'Number.isFinite', 'Number.isInteger', 'Number.isNaN',
        'Number.parseFloat', 'Number.parseInt', 'Number.toInteger',
      'Math',
        'Math.E', 'Math.LN10', 'Math.LN2', 'Math.LOG10E', 'Math.LOG2E',
        'Math.PI', 'Math.SQRT1_2', 'Math.SQRT2',
        'Math.abs', 'Math.acos', 'Math.acosh', 'Math.asin', 'Math.asinh',
        'Math.atan', 'Math.atan2', 'Math.atanh', 'Math.cbrt', 'Math.ceil',
        'Math.cos', 'Math.cosh', 'Math.exp', 'Math.exm1', 'Math.floor',
        'Math.fround', 'Math.hypot', 'Math.imul', 'Math.log', 'Math.log10',
        'Math.log1p', 'Math.log2', 'Math.max', 'Math.min', 'Math.pow',
        'Math.random', 'Math.round', 'Math.sign', 'Math.sin', 'Math.sinh',
        'Math.sqrt', 'Math.tan', 'Math.tanh', 'Math.trunc',
      'Date',
        'Date.UTC', 'Date.now', 'Date.parse',

      // Text processing
      'String',
        'String.fromCharCode', 'String.fromCodePoint',
      'RegExp',
        'RegExp.lastIndex',

      // Indexed collections
      'Array',
        'Array.isArray', 'Array.length',
      'Float32Array',
      'Float64Array',
      'Int16Array',
      'Int32Array',
      'Int8Array',
      'Uint16Array',
      'Uint32Array',
      'Uint8Array',
      'Uint8ClampedArray',
      'ParallelArray',

      // Keyed collections
      'Map',
      'Set',
      'WeakMap',
      'WeakSet',

      // Structured data
      'ArrayBuffer',
      'DataView',
      'JSON',
        'JSON.parse', 'JSON.stringify',

      // Control abstraction objects
      'Iterator',
      'Generator',
      'Promise',
        'Promise.all', 'Promise.accept', 'Promise.defer',
        'Promise.reject', 'Promise.race', 'Promise.resolve',

      // Reflection
      'Reflect',
      'Proxy',

      // Internationalization
      'Intl',
      'Intl.Collator',
      'Intl.DateTimeFormat',
      'Intl.NumberFormat',

      // Other
      'arguments',

      // Well-Known
      'console.assert', 'console.clear', 'console.log'
    ]);

    // Object.prototype properties
    [
      '.hasOwnProperty',
      '.length',
      '.isPrototypeOf',
      '.propertyIsEnumerable',
      '.prototype',
      '.toLocaleString',
      '.toString',
      '.unwatch',
      '.valueOf',
      '.watch',
    ].forEach(function(word) {
      map.set(word, 'keyword2');
    });
    return map;
  })()
});
