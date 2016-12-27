// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_JSDOC_TAGS_H_
#define JOANA_AST_JSDOC_TAGS_H_

namespace joana {
namespace ast {

// See [1] for tags supported by closure compiler.
// [1] src/com/google/javascript/jscomp/parsing/Annotation.java

// V(name, capital, syntax)
#define FOR_EACH_JSDOC_TAG_NAME(V)                   \
  V(author, Author, SingleLine)                      \
  V(const, Const, OptionalType)                      \
  V(constructor, Constructor, None)                  \
  V(define, Define, TypeDescription)                 \
  V(deprecated, Deprecated, Description)             \
  V(dict, Dict, None)                                \
  V(enum, Enum, Type)                                \
  V(export, Export, OptionalType)                    \
  V(extends, Extends, Type)                          \
  V(externs, Externs, None)                          \
  V(fileoverview, FileOverview, Description)         \
  V(final, Final, None)                              \
  V(implements, Implements, Type)                    \
  V(implicitCast, ImplicitCast, None)                \
  V(inheritDoc, InheritDoc, None)                    \
  V(interface, Interface, None)                      \
  V(lends, Lends, /* ObjectName */ Type)             \
  V(license, License, Description)                   \
  V(modifies, Modifies, /* {this|arguments} */ Type) \
  V(noalias, NoAlias, None)                          \
  V(nocollapse, NoCollapse, None)                    \
  V(nocompile, NoCompile, None)                      \
  V(nosideeffects, NoSideeffects, None)              \
  V(override, Override, None)                        \
  V(package, Package, None)                          \
  V(param, Param, TypeNameDescription)               \
  V(preserve, Preserve, Description)                 \
  V(private, Private, None)                          \
  V(protected, Protected, None)                      \
  V(public, Public, None)                            \
  V(record, Record, None)                            \
  V(return, Return, TypeDescription)                 \
  V(see, See, SingleLine)                            \
  V(struct, Struct, None)                            \
  V(suppress, Suppress, NameList)                    \
  V(template, Template, Names)                       \
  V(this, This, Type)                                \
  V(throws, Throws, Type)                            \
  V(type, Type, Type)                                \
  V(typedef, TypeDef, Type)                          \
  V(unrestricted, Unrestricted, None)

// V(capital, example)
#define FOR_EACH_JSDOC_TAG_SYNTAX(V)               \
  V(Description, "@deprecated description")        \
  V(NameList, "@suppress {name1, name2}")          \
  V(Names, "@template K, V")                       \
  V(None, "@constructor")                          \
  V(SingleLine, "@author ...")                     \
  V(OptionalType, "@const {type}")                 \
  V(Type, "@enum {type}")                          \
  V(TypeDescription, "@define {type} description") \
  V(TypeNameDescription, "@param {type} name description")

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_JSDOC_TAGS_H_
