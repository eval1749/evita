// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.JavaLexer = (function(options) {
  class JavaLexer extends ClikeLexer {
    /**
     * @param {!Document} document
     */
    constructor(document) {
      super(document, options);
    }
  }
  // TODO(eval1749): Once closure compiler support |static get|, we should use
  // it.
  Object.defineProperty(JavaLexer, 'keywords', {
    get: function() { return options.keywords; }
  });
  return JavaLexer;
})({
  useDot: true,
  characters: (function() {
    var attrs = ClikeLexer.newCharacters();
    // Include '@' as word character for attribute identifier coloring.
    attrs.set(Unicode.COMMERCIAL_AT, Lexer.NAMESTART_CHAR);
    return attrs;
  })(),
  keywords: Lexer.createKeywords([
  'abstract', 'assert',
  'boolean', 'break', 'byte',
  'case', 'catch', 'char', 'class', 'const', 'continue',
  'default', 'do', 'double',
  'else', 'enum', 'extends',
  'final', 'finally', 'float', 'for',
  'if',
  'goto',
  'implements', 'import', 'instanceof', 'int', 'interface',
  'long',
  'native', 'new',
  'package', 'private', 'protected', 'public',
  'return',
  'short', 'static', 'strictfp', 'super', 'switch', 'synchronized',
  'this', 'throw', 'throws', 'transient', 'try',
  'void', 'volatile',
  'while',

  // Reserved label
  'default:',

  // Literals
  'false', 'null', 'true',

  // Interfaces in java.lang
  'Appendable', 
  'CharSequence',
  'Cloneable',
  'Comparable',
  'Iterable',
  'Readable',
  'Runnable',
  'Thread.UncaughtExceptionHandler',

  // Classes in java.lang
  'Boolean',
  'Byte',
  'Character',
  'Character.Subset',
  'Character.Unicode',
  'Class',
  'ClassLoader',
  'Compiler',
  'Double',
  'Enum',
  'Float',
  'InheritableThreadLocal',
  'Integer',
  'Long',
  'Math',
  'Number',
  'Object',
  'Package',
  'Process',
  'ProcessBuilder',
  'Runtime',
  'RuntimePermission',
  'SecurityManager',
  'Short',
  'StackTraceElement',
  'StrictMath',
  'String',
  'StringBuffer',
  'StringBuilder',
  'System',
  'Thread',
  'ThreadGroup',
  'ThreadLocal',
  'Throwable',
  'Void',

  // Exceptions in java.lang
  'ArithmeticException',
  'ArrayIndexOutOfBoundsException',
  'ArrayStoreException',
  'ClassCastException',
  'ClassNotFoundException',
  'CloneNotSupportedException',
  'EnumConstantNotPresentException',
  'Exception',
  'IllegalAccessException',
  'IllegalArgumentException',
  'IllegalMonitorStateException',
  'IllegalStateException',
  'IllegalThreadStateException',
  'IndexOutOfBoundsException',
  'InstantiationException',
  'InterruptedException',
  'NegativeArraySizeException',
  'NoSuchFieldException',
  'NoSuchMethodException',
  'NullPointerException',
  'NumberFormatException',
  'RuntimeException',
  'SecurityException',
  'StringIndexOutOfBoundsException',
  'TypeNotPresentException',
  'UnsupportedOperationException',

  // Errors in java.lang
  'AbstractMethodError',
  'AssertionError',
  'ClassCircularityError',
  'ClassFormatError',
  'Error',
  'ExceptionInInitializerError',
  'IllegalAccessError',
  'IncompatibleClassChangeError',
  'InstantiationError',
  'InternalError',
  'LinkageError',
  'NoClassDefFoundError',
  'NoSuchFieldError',
  'NoSuchMethodError',
  'OutOfMemoryError',
  'StackOverflowError',
  'ThreadDeath',
  'UnknownError',
  'UnsatisfiedLinkError',
  'UnsupportedClassVersionError',
  'VerifyError',
  'VirtualMachineError',

  // Methods of java.lang.Object
  '.class',
  '.clone',
  '.equals',
  '.finalize',
  '.getClass',
  '.hashCode',
  '.notify',
  '.notifyAll',
  '.toString',
  '.wait',

  // Annotations
  '@Deprecated',
  '@Override',
  '@SuppressWarnings',
])});
