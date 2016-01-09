// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.IdlLexer = (function(options) {
  class IdlLexer extends global.ClikeLexer {
    /**
     * @param {!TextDocument} document
     */
    constructor(document) {
      super(document, options);
    }
  }
  // TODO(eval1749): Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(IdlLexer, 'keywords', {
    get: function() { return options.keywords; }
  });
  return IdlLexer;
})({
  characters: ClikeLexer.newCharacters(),
  keywords: Lexer.createKeywords([
    'any', 'attribute',
    'boolean', 'byte',
    'callback', 'const', 'creator',
    'Date','deleter', 'dictionary', 'DOMString', 'double',
    'enum',   'exception',
    'false', 'float',
    'getter',
    'implements', 'Infinity', 'inherit', 'interface',
    'legacycaller', 'long',
    'Nan', 'null',
    'object', 'octet', 'optional', 'or',
    'partial',
    'readonly',
    'sequence', 'setter', 'short', 'static', 'stringifier',
    'typedef', 'true',
    'unsigned', 'unrestricted',
    'void',

    // Reserved identifiers.
    'constructor', 'prototype', 'toString',

    // Extended attributes for ECMAScript
    'ArrayClass',
    'Clamp',
    'Constructor',
    'EnforceRange',
    'ImplicitThis',
    'LenientThis',
    'NamedConstructor', 'NamedPropertiesObject', 'NoInterfaceObject',
    'OverrideBuiltins',
    'PutForwards',
    'Replaceable',
    'TreatNonCallableAsNull', 'TreatNullAs', 'TreatUndefinedAs',
    'Unforgeable',

    // Extended attributes in Blink IDL
    'ImplementedAs'
])});
