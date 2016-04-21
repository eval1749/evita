// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const PythonTokenStateMachine = highlights.PythonTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticPythonKeywords = new Set();

class PythonPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    if (token.syntax !== 'identifier' || token.length == 1)
      return this.paintToken(token);
    /** @const @type {string} */
    const fullName = this.textOf(token);
    if (staticPythonKeywords.has(fullName))
      return this.paintToken2(token, 'keyword');
    const dotIndex = fullName.indexOf('.');
    if (dotIndex < 0 || !staticPythonKeywords.has(fullName.substr(0, dotIndex)))
      return this.paintToken(token);
    /** @const @type {number} */
    const nameEnd = token.start + dotIndex;
    this.setSyntax(token.start, nameEnd, 'keyword');
    this.setSyntax(nameEnd, token.end, 'identifier');
  }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new PythonPainter(document); }
}

class PythonHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, PythonPainter.create, new PythonTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticPythonKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticPythonKeywords.add(word); }
}

[
    // Keywords
    'and', 'as', 'assert', 'break', 'class', 'continue', 'def', 'del', 'elif',
    'else', 'except', 'exec', 'finally', 'for', 'from', 'global', 'if',
    'import', 'in', 'is', 'lambda', 'not', 'or', 'pass', 'print', 'raise',
    'return', 'try', 'while', 'with', 'yield',
    // Reserved classes of identifiers
    '_*', '__*__', '__*', '__builtin__',
    // Built-in Constants
    'Ellipsis', 'False', 'None', 'NotImplemented', 'True', '__debug__',
    // Built-in Types
    'complex', 'float', 'int', 'long', 'str', 'unicode', 'list', 'tuple',
    'bytearray', 'xrange', 'set', 'frozenset', 'dict',

    // 2 Built-in Functions
    '__import__', 'abs', 'all', 'any', 'basestring', 'bin', 'bool', 'bytearray',
    'callable', 'chr', 'classmethod', 'cmp', 'compile', 'complex', 'delattr',
    'dict', 'dir', 'divmod', 'enumerate', 'eval', 'execfile', 'file', 'filter',
    'float', 'format', 'frozenset', 'getattr', 'globals', 'hasattr', 'hash',
    'help', 'hex', 'id', 'input', 'int', 'isinstance', 'issubclass', 'iter',
    'len', 'list', 'locals', 'long', 'map', 'max', 'memoryview', 'min', 'next',
    'object', 'oct', 'open', 'ord', 'pow', 'print', 'property', 'range',
    'raw_input', 'reduce', 'reload', 'repr', 'reversed', 'round', 'set',
    'setattr', 'slice', 'sorted', 'staticmethod', 'str', 'sum', 'super',
    'tuple', 'type', 'unichr', 'unicode', 'vars', 'xrange', 'zip',

    // 3.1 Objects, values and types
    '__*item__', '__*slice__', '__abs__', '__add__', '__and__', '__bases__',
    '__call__', '__class__', '__closure__', '__cmp__', '__code__', '__coerce__',
    '__complex__', '__contains__', '__defaults__', '__del__', '__delattr__',
    '__delete__', '__delitem__', '__delslice__', '__dict__', '__div__',
    '__divmod__', '__doc__', '__enter__', '__eq__', '__exit__', '__file__',
    '__float__', '__floordiv__', '__func__', '__future__', '__ge__', '__get__',
    '__getattr__', '__getattribute__', '__getitem__', '__getslice__',
    '__globals__', '__gt__', '__hash__', '__hex__', '__iadd__', '__iand__',
    '__idiv__', '__ifloordiv__', '__ilshift__', '__imod__', '__imul__',
    '__index__', '__init__', '__instancecheck__', '__int__', '__invert__',
    '__iop__', '__ior__', '__ipow__', '__irshift__', '__isub__', '__iter__',
    '__itruediv__', '__ixor__', '__le__', '__len__', '__long__', '__lshift__',
    '__lt__', '__metaclass__', '__missing__', '__mod__', '__module__',
    '__mro__', '__mul__', '__name__', '__ne__', '__neg__', '__new__',
    '__nonzero__', '__oct__', '__op__', '__or__', '__pos__', '__pow__',
    '__radd__', '__rand__', '__rcmp__', '__rdiv__', '__rdivmod__', '__repr__',
    '__reversed__', '__rfloordiv__', '__rlshift__', '__rmod__', '__rmul__',
    '__rop__', '__ror__', '__rpow__', '__rrshift__', '__rshift__', '__rsub__',
    '__rtruediv__', '__rxor__', '__self__', '__set__', '__setattr__',
    '__setitem__', '__setslice__', '__slots__', '__str__', '__sub__',
    '__subclasscheck__', '__truediv__', '__unicode__', '__weakref__', '__xor__',

    // 5.13. Special Attributes
    '__dict__', '__class__', '__bases__', '__name__', '__mro__',
    '__subclasses__',

    // 6 Built-in Exceptions
    'ArithmeticError', 'AssertionError', 'AttributeError', 'BaseException',
    'BufferError', 'BytesWarning', 'DeprecationWarning', 'EnvironmentError',
    'EOFError', 'Exception', 'FloatingPointError', 'FutureWarning',
    'GeneratorExit', 'ImportError', 'ImportWarning', 'IndentationError',
    'IndexError', 'IOError', 'KeyboardInterrupt', 'KeyError', 'LookupError',
    'MemoryError', 'NameError', 'NotImplementedError', 'OSError',
    'OverflowError', 'PendingDeprecationWarning', 'ReferenceError',
    'RuntimeError', 'RuntimeWarning', 'StandardError', 'StopIteration',
    'SyntaxError', 'SyntaxWarning', 'SystemError', 'SystemExit', 'TabError',
    'TypeError', 'UnboundLocalError', 'UnicodeDecodeError',
    'UnicodeEncodeError', 'UnicodeError', 'UnicodeTranslateError',
    'UnicodeWarning', 'UserWarning', 'ValueError', 'Warning', 'WindowsError',
    'ZeroDivisionError',

    // 28.5 __main__ -- Top-level script environment
    '__main__',

    // Commonly used word
    'self',
].forEach(keyword => PythonHighlighter.addKeyword(keyword));

/** @constructor */
highlights.PythonHighlighter = PythonHighlighter;

/** @constructor */
highlights.PythonPainter = PythonPainter;
});

// Override |PythonLexer| by |PythonHighlighter|.
// TODO(eval1749): Once we get rid of |PythonLexer|, we should get rid of this
// override.
global['PythonLexer'] = highlights.PythonHighlighter;
