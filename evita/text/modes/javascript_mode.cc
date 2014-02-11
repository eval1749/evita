// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/javascript_mode.h"

#include <memory>

#include "common/memory/singleton.h"
#include "evita/text/modes/clike_lexer.h"
#include "evita/text/modes/mode.h"

#define DEBUG_LEXER 0

namespace text {
namespace {
//////////////////////////////////////////////////////////////////////
//
// JavaScript Keywords
//
const char16* k_rgpwszJavaScriptKeyword[] = {
  // Operator
  L"+", L"-", L"*", L"/", L"%", L"&", L"^", L"|",
  L"+=", L"-=", L"*=", L"/=", L"%=", L"&=", L"^=", L"|=",
  L"&&", L"||",
  L"++", L"--",
  L"<<", L">>", L"<<=", L">>=",
  L"==", L"!=", L"<", L"<=", L">", L">=",
  L"!", L"~",
  L"(", L")", L"[", L"]", L"{", L"}",

  L"break",
  L"case",
  L"catch",
  L"class",
  L"const",
  L"continue",
  L"debugger",
  L"default",
  L"delete",
  L"do",
  L"else",
  L"export",
  L"extends",
  L"finally",
  L"for",
  L"function",
  L"if",
  L"import",
  L"in",
  L"instanceof",
  L"new",
  L"return",
  L"super",
  L"switch",
  L"this",
  L"throw",
  L"try",
  L"typeof",
  L"var",
  L"void",
  L"while",
  L"with",
  L"yield",

  // Literals
  L"false",
  L"null",
  L"true",

  // Future reserved word in ES6
  L"enum",
  L"implements",
  L"interface",
  L"package",
  L"private",
  L"protected",
  L"public",
  L"startic",

  // Standard build-int objects
  // Value properties
  L"Infinity",
  L"NaN",
  L"undefined",

  // Function properties
  L"eval",
  L"uneval L",
  L"isFinite",
  L"isNaN",
  L"parseFloat",
  L"parseInt",
  L"decodeURI",
  L"decodeURIComponent",
  L"encodeURI",
  L"encodeURIComponent",
  L"escape L",
  L"unescape",

  // Fundamental objects
  L"Object",
    L"Object.create", L"Object.defineProperties", L"Object.defineProperty",
    L"Object.freeze", L"Object.getOwnPropertyDescriptor",
    L"Object.getOwnPropertyNames", L"Object.getPrototypeOf", L"Object.is",
    L"Object.isExtensible", L"Object.isFrozen", L"Object.isSealed",
    L"Object.keys", L"Object.preventExtensions", L"Object.seal",
    L"Object.setPrototypeOf",
  L"Function",
  L"Boolean",
  L"Symbol ",
  L"Error",
  L"EvalError",
  L"InternalError",
  L"RangeError",
  L"ReferenceError",
  L"StopIteration",
  L"SyntaxError",
  L"TypeError",
  L"URIError",

  // Numbers and dates
  L"Number",
    L"Number.EPSILON", L"Number.MAX_VALUE", L"Number.MIN_VALUE",
    L"Number.NEGATIVE_INFINITY", L"Number.NaN", L"Number.POSITIVE_INFINITY",
    L"Number.isFinite", L"Number.isInteger", L"Number.isNaN",
    L"Number.parseFloat", L"Number.parseInt", L"Number.toInteger",
  L"Math",
    L"Math.E", L"Math.LN10", L"Math.LN2", L"Math.LOG10E", L"Math.LOG2E",
    L"Math.PI", L"Math.SQRT1_2", L"Math.SQRT2",
    L"Math.abs", L"Math.acos", L"Math.acosh", L"Math.asin", L"Math.asinh",
    L"Math.atan", L"Math.atan2", L"Math.atanh", L"Math.cbrt", L"Math.ceil",
    L"Math.cos", L"Math.cosh", L"Math.exp", L"Math.exm1", L"Math.floor",
    L"Math.fround", L"Math.hypot", L"Math.imul", L"Math.log", L"Math.log10",
    L"Math.log1p", L"Math.log2", L"Math.max", L"Math.min", L"Math.pow",
    L"Math.random", L"Math.round", L"Math.sign", L"Math.sin", L"Math.sinh",
    L"Math.sqrt", L"Math.tan", L"Math.tanh", L"Math.trunc",
  L"Date",
    L"Date.UTC", L"Date.now", L"Date.parse",

  // Text processing
  L"String",
    L"String.fromCharCode", L"String.fromCodePoint",
  L"RegExp",
    L"RegExp.lastIndex",

  // Indexed collections
  L"Array",
    L"Array.isArray", L"Array.length",
  L"Float32Array",
  L"Float64Array",
  L"Int16Array",
  L"Int32Array",
  L"Int8Array",
  L"Uint16Array",
  L"Uint32Array",
  L"Uint8Array",
  L"Uint8ClampedArray",
  L"ParallelArray",

  // Keyed collections
  L"Map",
  L"Set",
  L"WeakMap",
  L"WeakSet",

  // Structured data
  L"ArrayBuffer",
  L"DataView",
  L"JSON",
    L"JSON.parse", L"JSON.stringify",

  // Control abstraction objects
  L"Iterator",
  L"Generator",
  L"Promise",
    L"Promise.all", L"Promise.cast", L"Promise.race", L"Promise.reject",

  // Reflection
  L"Reflect",
  L"Proxy",

  // Internationalization
  L"Intl",
  L"Intl.Collator",
  L"Intl.DateTimeFormat",
  L"Intl.NumberFormat",

  // Other
  L"arguments",
  L"arguments.length",

  // Object.prototype properties
  L".prototype",
  L".hasOwnProperty",
  L".isPrototypeOf",
  L".propertyIsEnumerable",
  L".toLocaleString",
  L".toString",
  L".unwatch",
  L".valueOf",
  L".watch",
};

//////////////////////////////////////////////////////////////////////
//
// JavaScriptKeywordTable
//
class JavaScriptKeywordTable :
    public common::Singleton<JavaScriptKeywordTable>,
    public NewLexer::KeywordTable {
  DECLARE_SINGLETON_CLASS(JavaScriptKeywordTable);

  private: JavaScriptKeywordTable() {
    AddKeywords(k_rgpwszJavaScriptKeyword, arraysize(k_rgpwszJavaScriptKeyword));
  }

  public: ~JavaScriptKeywordTable() = default;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptKeywordTable);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// JavaScriptLexer
//
class JavaScriptLexer : public ClikeLexer {
  public: JavaScriptLexer(Buffer* pBuffer)
    : ClikeLexer(pBuffer, JavaScriptKeywordTable::instance(), '\0') {
  }

  public: ~JavaScriptLexer() = default;

  DISALLOW_COPY_AND_ASSIGN(JavaScriptLexer);
};

//////////////////////////////////////////////////////////////////////
//
// JavaScriptMode
//
JavaScriptMode::JavaScriptMode() {
}

JavaScriptMode::~JavaScriptMode() {
}

// Mode
const char16* JavaScriptMode::GetName() const {
  return L"JavaScript";
}

// ModeWithLexer
Lexer* JavaScriptMode::CreateLexer(Buffer* buffer) {
  return new JavaScriptLexer(buffer);
}

} // namespace text
