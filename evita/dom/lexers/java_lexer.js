// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.JavaLexer = (function(options) {
  /**
   * @constructor
   * @extends Lexer
   * @param {!Document} document
   */
  function JavaLexer(document) {
    ClikeLexer.call(this, document, options);
  }

  JavaLexer.prototype = Object.create(ClikeLexer.prototype, {
    constructor: {value: JavaLexer},
  });

  return JavaLexer;
})({
  useDot: true,
  keywords: [
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
]});
