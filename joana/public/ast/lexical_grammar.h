// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_LEXICAL_GRAMMAR_H_
#define JOANA_PUBLIC_AST_LEXICAL_GRAMMAR_H_

namespace joana {

#define FOR_EACH_CHARACTER_NAME(V)                   \
  V(0x0009, Tab, TAB)                                \
  V(0x000A, LineFeed, LINE_FEED)                     \
  V(0x000B, LineTab, LINE_TAB)                       \
  V(0x000C, FormFeed, FORM_FEED)                     \
  V(0x000D, CarriageReturn, CARRIAGE_RETURN)         \
  V(0x0020, Space, SPACE)                            \
  V(0x00A0, NonBreakSpace, NON_BREAK_SPACE)          \
  V(0x2028, LineSeparator, LINE_SEPARATOR)           \
  V(0x2029, ParagraphSeparator, PARAGRAPH_SEPARATOR) \
  V(0xFEFF, ZeroWidthNoBreakSpace, ZERO_WIDT_HNO_BREAK_SPACE)

#define FOR_EACH_JAVASCRIPT_TOKEN(V)        \
  V(IdentifierName, IDENTIFIER_NAME)        \
  V(MultiLineComment, MULTI_LINE_COMMENT)   \
  V(SingleLineComment, SINGLE_LINE_COMMENT) \
  V(Punctuator, PUNCTUATOR)                 \
  V(NumericLiteral, NUMERIC_LITERAL)        \
  V(StringLiteral, STRING_LITERAL)          \
  V(Template, TEMPLATE)

// Note: true, false and null are not actual keywords.
#define FOR_EACH_JAVASCRIPT_KEYWORD(V)   \
  V(async, Async, ASYNC)                 \
  V(await, Await, AWAIT)                 \
  V(break, Break, BREAK)                 \
  V(case, Case, CASE)                    \
  V(catch, Catch, CATCH)                 \
  V(class, Class, CLASS)                 \
  V(const, Const, CONST)                 \
  V(continue, Continue, CONTINUE)        \
  V(debugger, Debugger, DEBUGGER)        \
  V(default, Default, DEFAULT)           \
  V(delete, Delete, DELETE)              \
  V(do, Do, DO)                          \
  V(else, Else, ELSE)                    \
  V(enum, Enum, ENUM)                    \
  V(export, Export, EXPORT)              \
  V(extends, Extends, EXTENDS)           \
  V(false, False, FALSE)                 \
  V(finally, Finally, FINALLY)           \
  V(for, For, FOR)                       \
  V(function, Function, FUNCTION)        \
  V(if, If, IF)                          \
  V(implements, Implements, IMPLEMENTS)  \
  V(import, Import, IMPORT)              \
  V(in, In, IN)                          \
  V(instanceof, InstanceOf, INSTANCE_OF) \
  V(interface, Interface, INTERFACE)     \
  V(let, Let, LET)                       \
  V(new, New, NEW)                       \
  V(null, Null, NULL)                    \
  V(package, Package, PACKAGE)           \
  V(private, Private, PRIVATE)           \
  V(protected, Protected, PROTECTED)     \
  V(public, Public, PUBLIC)              \
  V(return, Return, RETURN)              \
  V(static, Static, STATIC)              \
  V(super, Super, SUPER)                 \
  V(switch, Switch, SWITCH)              \
  V(this, This, THIS)                    \
  V(throw, Throw, THROW)                 \
  V(true, True, TRUE)                    \
  V(try, Try, TRY)                       \
  V(typeof, TypeOf, TYPE_OF)             \
  V(var, Var, VAR)                       \
  V(void, Void, VOID)                    \
  V(while, While, WHILE)                 \
  V(with, With, WITH)                    \
  V(yield, Yield, YIELD)

#define FOR_EACH_JAVASCRIPT_KNOWN_WORD(V)  \
  V(Object, Object, OBJECT)                \
  V(constructor, Constructor, CONSTRUCTOR) \
  V(from, From, CONSTRUCTOR)               \
  V(of, Of, OF)                            \
  V(prototype, Prototype, PROTOTYPE)       \
  V(undefined, Undefined, UNDEFINED)

#define FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)                       \
  V("???", Invalid, INVALID)                                    \
  V("{", LeftBrace, LEFT_BRACE)                                 \
  V("}", RightBrace, RIGHT_BRACE)                               \
  V("[", LeftBracket, LEFT_BRACKET)                             \
  V("]", RightBracket, RIGHT_BRACKET)                           \
  V("(", LeftParenthesis, LEFT_PARENTHESIS)                     \
  V(")", RightParenthesis, RIGHT_PARENTHESIS)                   \
  V(".", Dot, DOT)                                              \
  V("...", DotDotDot, DOT_DOT_DOT)                              \
  V(";", SemiColon, SEMI_COLON)                                 \
  V(",", Comma, COMMA)                                          \
  V("<", LessThan, LEFT_THAN)                                   \
  V("<=", LessThanOrEqual, LESS_THAN_OR_EQUAL)                  \
  V(">", GreaterThan, GREATER_THAN)                             \
  V(">=", GreaterThanOrEqual, GREATER_THAN_OR_EQUAL)            \
  V("==", EqualEqual, EQUAL_EQUAL)                              \
  V("===", EqualEqualEqual, EQUAL_EQUAL_EQUAL)                  \
  V("+", Plus, PLUS)                                            \
  V("-", Minus, MIUS)                                           \
  V("*", Times, TIMES)                                          \
  V("%", Modulo, MODULO)                                        \
  V("++", PlusPlus, PLUS_PLUS)                                  \
  V("--", MinusMinus, MINUS_MINUS)                              \
  V("<<", LeftShift, LEFT_SHIFT)                                \
  V(">>", RightShift, RIGHT_SHIFT)                              \
  V(">>>", UnsignedRightShift, UNSIGNED_RIGHT_SHIFT)            \
  V("&", BitAnd, BIT_AND)                                       \
  V("|", BitOr, BIT_OR)                                         \
  V("|", BitXor, BIT_XOR)                                       \
  V("!", LogicalNot, LOGICAL_NOT)                               \
  V("!=", NotEqual, NOT_EQUAL)                                  \
  V("!==", NotEqualEqual, NOT_EQUAL_EQUAL)                      \
  V("~", BitNot, BIT_NOT)                                       \
  V("&&", LogicalAnd, LOGICAL_AND)                              \
  V("||", LogicalOr, LOGICAL_OR)                                \
  V("?", Question, QUESTION)                                    \
  V(":", Colon, COLON)                                          \
  V("=", Equal, EQUAL)                                          \
  V("+=", PlusEqual, PLUS_EQUAL)                                \
  V("-=", MinusEqual, PLUS_EQUAL)                               \
  V("*=", TimesEqual, TIMES_EQUAL)                              \
  V("%=", ModuloEqual, MODULO_EQUAL)                            \
  V("<<=", LeftShiftEqual, LEFT_SHIT_EQUAL)                     \
  V(">>=", RightShiftEqual, RIGHT_SHIT_EQUAL)                   \
  V(">>>=", UnsignedRightShiftEqual, UNSIGNED_RIGHT_SHIT_EQUAL) \
  V("&=", BitAndEqual, BIT_AND_EQUAL)                           \
  V("|=", BitOrEqual, BIT_OR_EQUAL)                             \
  V("^=", BitXorEqual, BIT_XOR_EQUAL)                           \
  V("=>", Arrow, ARROW)                                         \
  V("**", TimesTimes, TIMES_TIMES)                              \
  V("**=", TimesTimesEqual, TIMES_TIMES_EQUAL)                  \
  V("/", Divide, DIVIDE)                                        \
  V("/=", DivideEqual, DIVIDE_EQUAL)

}  // namespace joana

#endif  // JOANA_PUBLIC_AST_LEXICAL_GRAMMAR_H_
