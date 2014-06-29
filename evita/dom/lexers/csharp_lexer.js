// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.CsharpLexer = (function(options) {
  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function CsharpLexer(document) {
    ClikeLexer.call(this, document, options);
  }

  CsharpLexer.keywords = options.keywords;
  CsharpLexer.prototype = Object.create(ClikeLexer.prototype, {
    constructor: {value: CsharpLexer},
  });

  return CsharpLexer;
})({
  useDot: true,
  characters: ClikeLexer.newCharacters(),
  keywords: Lexer.createKeywords([
    'abstract', 'as',
    'base', 'bool', 'break', 'byte',
    'case', 'catch', 'char', 'checked', 'class', 'const', 'continue',
    'decimal', 'default', 'delegate', 'do', 'double',
    'else', 'enum', 'event', 'explicit', 'extern',
    'false', 'finally', 'fixed', 'float', 'for', 'foreach',
    'goto',
    'if', 'implicit', 'in', 'int', 'interface', 'internal', 'is',
    'lock', 'long',
    'namespace', 'new', 'null',
    'object', 'operator', 'out', 'override',
    'params', 'private', 'protected', 'public',
    'readonly', 'ref', 'return',
    'sbyte', 'sealed', 'short', 'sizeof', 'stackalloc', 'static', 'string',
    'struct', 'switch',
    'this', 'throw', 'true', 'try', 'typeof',
    'uint', 'ulong', 'unchecked', 'unsafe', 'ushort', 'using',
    'virtual', 'void', 'volatile',
    'while',

    // Contextual keywords
    'add', 'alias', 'ascending', 'async', 'await',
    'descending', 'dynamic',
    'from',
    'get', 'global', 'group',
    'into',
    'join',
    'let',
    'orderby',
    'partial',
    'remove',
    'select', 'set',
    'value', 'var',
    'where',
    'yield'
])});
