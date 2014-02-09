// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/java_mode.h"

#include "evita/text/modes/clike_lexer.h"
#include "evita/text/modes/mode.h"

#define DEBUG_LEXER 0

namespace text {
namespace {
//////////////////////////////////////////////////////////////////////
//
// Java Keywords
//
const char16* k_rgpwszJavaKeyword[] = {
  // Operator
  L"+", L"-", L"*", L"/", L"%", L"&", L"^", L"|",
  L"+=", L"-=", L"*=", L"/=", L"%=", L"&=", L"^=", L"|=",
  L"&&", L"||",
  L"++", L"--",
  L"<<", L">>", L"<<=", L">>=",
  L"==", L"!=", L"<", L"<=", L">", L">=",
  L"!", L"~",
  L"(", L")", L"[", L"]", L"{", L"}",

  L"abstract",
  L"assert", // 3rd
  L"boolean",
  L"break",
  L"byte",
  L"case",
  L"catch",
  L"char",
  L"class",
  L"const",
  L"continue",
  L"default",
  L"do",
  L"double",
  L"else",
  L"enum", // 3rd
  L"extends",
  L"final",
  L"finally",
  L"float",
  L"for",
  L"if",
  L"goto",
  L"implements",
  L"import",
  L"instanceof",
  L"int",
  L"interface",
  L"long",
  L"native",
  L"new",
  L"package",
  L"private",
  L"protected",
  L"public",
  L"return",
  L"short",
  L"static",
  L"strictfp",
  L"super",
  L"switch",
  L"synchronized",
  L"this",
  L"throw",
  L"throws",
  L"transient",
  L"try",
  L"void",
  L"volatile",
  L"while",

  // Literals
  L"false",
  L"null",
  L"true",

  // Interfaces in java.lang
  L"Appendable",
  L"CharSequence",
  L"Cloneable",
  L"Comparable",
  L"Iterable",
  L"Readable",
  L"Runnable",
  L"Thread.UncaughtExceptionHandler",

  // Classes in java.lang
  L"Boolean",
  L"Byte",
  L"Character",
  L"Character.Subset",
  L"Character.Unicode",
  L"Class",
  L"ClassLoader",
  L"Compiler",
  L"Double",
  L"Enum",
  L"Float",
  L"InheritableThreadLocal",
  L"Integer",
  L"Long",
  L"Math",
  L"Number",
  L"Object",
  L"Package",
  L"Process",
  L"ProcessBuilder",
  L"Runtime",
  L"RuntimePermission",
  L"SecurityManager",
  L"Short",
  L"StackTraceElement",
  L"StrictMath",
  L"String",
  L"StringBuffer",
  L"StringBuilder",
  L"System",
  L"Thread",
  L"ThreadGroup",
  L"ThreadLocal",
  L"Throwable",
  L"Void",

  // Exceptions in java.lang
  L"ArithmeticException",
  L"ArrayIndexOutOfBoundsException",
  L"ArrayStoreException",
  L"ClassCastException",
  L"ClassNotFoundException",
  L"CloneNotSupportedException",
  L"EnumConstantNotPresentException",
  L"Exception",
  L"IllegalAccessException",
  L"IllegalArgumentException",
  L"IllegalMonitorStateException",
  L"IllegalStateException",
  L"IllegalThreadStateException",
  L"IndexOutOfBoundsException",
  L"InstantiationException",
  L"InterruptedException",
  L"NegativeArraySizeException",
  L"NoSuchFieldException",
  L"NoSuchMethodException",
  L"NullPointerException",
  L"NumberFormatException",
  L"RuntimeException",
  L"SecurityException",
  L"StringIndexOutOfBoundsException",
  L"TypeNotPresentException",
  L"UnsupportedOperationException",

  // Errors in java.lang
  L"AbstractMethodError",
  L"AssertionError",
  L"ClassCircularityError",
  L"ClassFormatError",
  L"Error",
  L"ExceptionInInitializerError",
  L"IllegalAccessError",
  L"IncompatibleClassChangeError",
  L"InstantiationError",
  L"InternalError",
  L"LinkageError",
  L"NoClassDefFoundError",
  L"NoSuchFieldError",
  L"NoSuchMethodError",
  L"OutOfMemoryError",
  L"StackOverflowError",
  L"ThreadDeath",
  L"UnknownError",
  L"UnsatisfiedLinkError",
  L"UnsupportedClassVersionError",
  L"VerifyError",
  L"VirtualMachineError",

  // Methods of java.lang.Object
  #if 0
      L".clone",
      L".equals",
      L".finalize",
      L".getClass",
      L".hashCode",
      L".notify",
      L".notifyAll",
      L".toString",
      L".wait",
  #endif

  // Annotations
  L"@Deprecated",
  L"@Override",
  L"@SuppressWarnings",
};

//////////////////////////////////////////////////////////////////////
//
// JavaKeywordTable
//
class JavaKeywordTable :
    public common::Singleton<JavaKeywordTable>, public NewLexer::KeywordTable {
  DECLARE_SINGLETON_CLASS(JavaKeywordTable);

  private: JavaKeywordTable() {
    AddKeywords(k_rgpwszJavaKeyword, arraysize(k_rgpwszJavaKeyword));
  }

  public: ~JavaKeywordTable() = default;

  DISALLOW_COPY_AND_ASSIGN(JavaKeywordTable);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// JavaLexer
//
class JavaLexer : public ClikeLexer {
  public: JavaLexer(Buffer* pBuffer)
    : ClikeLexer(pBuffer, JavaKeywordTable::instance(), '@') {
  }

  public: ~JavaLexer() = default;

  DISALLOW_COPY_AND_ASSIGN(JavaLexer);
};

//////////////////////////////////////////////////////////////////////
//
// JavaMode
//
class JavaMode : public Mode {
  private: JavaLexer m_oLexer;

  // ctor/dtor
  public: JavaMode(ModeFactory* pFactory, Buffer* pBuffer)
      : Mode(pFactory, pBuffer), m_oLexer(pBuffer) {
  }

  // [D]
  public: virtual bool DoColor(Count lCount) override {
    return m_oLexer.Run(lCount);
  }

  DISALLOW_COPY_AND_ASSIGN(JavaMode);
};

//////////////////////////////////////////////////////////////////////
//
// JavaModeFactory
//
JavaModeFactory::JavaModeFactory() {
}

JavaModeFactory::~JavaModeFactory() {
}

Mode* JavaModeFactory::Create(Buffer* pBuffer) {
  return new JavaMode(this, pBuffer);
}

} // namespace text
