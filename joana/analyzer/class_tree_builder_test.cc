// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "joana/analyzer/analyzer_test_base.h"

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/analyzer/class_tree_builder.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/values.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code_factory.h"

namespace joana {
namespace analyzer {

namespace {

const char* const kSourceCode = "A B C D E F G T";

std::vector<Class*> ClassListOf(const Class& clazz) {
  std::vector<Class*> class_list;
  for (auto& element : clazz.class_list())
    class_list.push_back(&element);
  return class_list;
}

template <typename... Args>
std::vector<Class*> ClassListFrom(const Args&... classes) {
  return std::vector<Class*>{const_cast<Class*>(&classes)...};
}

const SourceCode& NewSourceCode(Zone* zone) {
  SourceCode::Factory source_code_factory(zone);
  const auto& text16 = base::UTF8ToUTF16(kSourceCode);
  const auto size = text16.size() * sizeof(base::char16);
  auto* data = static_cast<base::char16*>(zone->Allocate(size));
  ::memcpy(data, text16.data(), size);
  const auto& source_code = source_code_factory.New(
      base::FilePath(), base::StringPiece16(data, text16.size()));
  return source_code;
}

}  // namespace

class ClassTreeBuilderTest : public AnalyzerTestBase {
 protected:
  ClassTreeBuilderTest();
  ~ClassTreeBuilderTest() override = default;

  Context& analyze_context() { return *context_; }
  TypeFactory& type_factory() { return context_->type_factory(); }

  std::string GetErrors() const;

  Class& NewClass(base::StringPiece name_piece);
  Class& NewConstructedClass(Class* generic_class,
                             const std::vector<const Type*>& parameters);
  Class& NewGenericClass(base::StringPiece name_piece,
                         const std::vector<const TypeParameter*>& parameters);
  const ast::Node& NewName(base::StringPiece name_piece);
  const TypeParameter& NewTypeParameter(base::StringPiece name_piece);

 private:
  const std::unique_ptr<Context> context_;
  Zone zone_;
  ast::NodeFactory node_factory_;
  const SourceCode& source_code_;

  DISALLOW_COPY_AND_ASSIGN(ClassTreeBuilderTest);
};

ClassTreeBuilderTest::ClassTreeBuilderTest()
    : context_(NewContext()),
      zone_("ClassTreeBuilderTest"),
      node_factory_(&zone_),
      source_code_(NewSourceCode(&zone_)) {}

std::string ClassTreeBuilderTest::GetErrors() const {
  std::ostringstream ostream;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

Class& ClassTreeBuilderTest::NewClass(base::StringPiece name_piece) {
  const auto& name = NewName(name_piece);
  auto& properties = context_->factory().NewProperties(name);
  return context_->factory().NewNormalClass(ClassKind::Class, name, name,
                                            &properties);
}

Class& ClassTreeBuilderTest::NewConstructedClass(
    Class* generic_class,
    const std::vector<const Type*>& arguments) {
  return context_->factory().NewConstructedClass(
      &generic_class->As<GenericClass>(), arguments);
}

Class& ClassTreeBuilderTest::NewGenericClass(
    base::StringPiece name_piece,
    const std::vector<const TypeParameter*>& parameters) {
  const auto& name = NewName(name_piece);
  auto& properties = context_->factory().NewProperties(name);
  return context_->factory().NewGenericClass(ClassKind::Class, name, name,
                                             parameters, &properties);
}

Class& NewGenericClass(base::StringPiece name_piece,
                       const std::vector<TypeParameter*>& parameters);

const ast::Node& ClassTreeBuilderTest::NewName(base::StringPiece name_piece) {
  const auto start = base::StringPiece(kSourceCode).find(name_piece);
  DCHECK_NE(start, base::StringPiece::npos) << name_piece;
  return node_factory_.NewName(source_code_.Slice(
      static_cast<int>(start), static_cast<int>(start + name_piece.size())));
}

const TypeParameter& ClassTreeBuilderTest::NewTypeParameter(
    base::StringPiece name_piece) {
  const auto& name = NewName(name_piece);
  return type_factory().NewTypeParameter(name).As<TypeParameter>();
}

TEST_F(ClassTreeBuilderTest, Basic) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  auto& class_c = NewClass("C");
  Value::Editor().SetClassHeritage(&class_b, {&class_a});
  Value::Editor().SetClassHeritage(&class_c, {&class_b});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Process(&class_b);
  builder.Process(&class_c);
  builder.Build();

  EXPECT_EQ("", GetErrors());
  EXPECT_EQ(ClassListFrom(class_a), ClassListOf(class_a));
  EXPECT_EQ(ClassListFrom(class_b, class_a), ClassListOf(class_b));
  EXPECT_EQ(ClassListFrom(class_c, class_b, class_a), ClassListOf(class_c));
}

TEST_F(ClassTreeBuilderTest, Basic2) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  auto& class_c = NewClass("C");
  Value::Editor().SetClassHeritage(&class_c, {&class_a, &class_b});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Process(&class_b);
  builder.Process(&class_c);
  builder.Build();

  EXPECT_EQ("", GetErrors());
  EXPECT_EQ(ClassListFrom(class_a), ClassListOf(class_a));
  EXPECT_EQ(ClassListFrom(class_b), ClassListOf(class_b));
  EXPECT_EQ(ClassListFrom(class_c, class_a, class_b), ClassListOf(class_c));
}

TEST_F(ClassTreeBuilderTest, ConstructedClass) {
  auto& class_a = NewClass("A");
  auto& generic_class = NewGenericClass("G", {&NewTypeParameter("T")});
  auto& constructed_class = NewConstructedClass(
      &generic_class, {&type_factory().NewClassType(&class_a)});
  auto& class_b = NewClass("B");
  Value::Editor().SetClassHeritage(&class_b, {&constructed_class});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Process(&class_b);
  builder.Process(&generic_class);
  builder.Build();

  EXPECT_EQ("", GetErrors());
  EXPECT_EQ(ClassListFrom(class_a), ClassListOf(class_a));
  EXPECT_EQ(ClassListFrom(class_b, constructed_class), ClassListOf(class_b));
}

TEST_F(ClassTreeBuilderTest, Diamond) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  auto& class_c = NewClass("C");
  auto& class_d = NewClass("D");
  Value::Editor().SetClassHeritage(&class_b, {&class_a});
  Value::Editor().SetClassHeritage(&class_c, {&class_a});
  Value::Editor().SetClassHeritage(&class_d, {&class_b, &class_c});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Process(&class_b);
  builder.Process(&class_c);
  builder.Process(&class_d);
  builder.Build();

  EXPECT_EQ("", GetErrors());
  EXPECT_EQ(ClassListFrom(class_a), ClassListOf(class_a));
  EXPECT_EQ(ClassListFrom(class_b, class_a), ClassListOf(class_b));
  EXPECT_EQ(ClassListFrom(class_c, class_a), ClassListOf(class_c));
  EXPECT_EQ(ClassListFrom(class_d, class_b, class_c, class_a),
            ClassListOf(class_d));
}

TEST_F(ClassTreeBuilderTest, ErrorCycle) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  auto& class_c = NewClass("C");
  Value::Editor().SetClassHeritage(&class_a, {&class_c});
  Value::Editor().SetClassHeritage(&class_b, {&class_a});
  Value::Editor().SetClassHeritage(&class_c, {&class_b});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Process(&class_b);
  builder.Process(&class_c);
  builder.Build();

  EXPECT_EQ(
      "ANALYZER_ERROR_CLASS_TREE_CYCLE@0:3\n"
      "ANALYZER_ERROR_CLASS_TREE_CYCLE@0:5\n"
      "ANALYZER_ERROR_CLASS_TREE_CYCLE@2:5\n",
      GetErrors())
      << "Cycle of A->B-C->A";
}

TEST_F(ClassTreeBuilderTest, ErrorSelf) {
  auto& class_a = NewClass("A");
  Value::Editor().SetClassHeritage(&class_a, {&class_a});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_a);
  builder.Build();

  EXPECT_EQ("ANALYZER_ERROR_CLASS_TREE_CYCLE@0:1\n", GetErrors())
      << "class A extends A {}";
}

TEST_F(ClassTreeBuilderTest, ErrorUndefined) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  Value::Editor().SetClassHeritage(&class_b, {&class_a});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_b);
  builder.Build();

  EXPECT_EQ("ANALYZER_ERROR_CLASS_TREE_UNDEFINED_CLASS@0:1\n", GetErrors())
      << "class B extends A, but class A is not defined.";
}

TEST_F(ClassTreeBuilderTest, ForwardRefernce) {
  auto& class_a = NewClass("A");
  auto& class_b = NewClass("B");
  auto& class_c = NewClass("C");
  Value::Editor().SetClassHeritage(&class_b, {&class_a});
  Value::Editor().SetClassHeritage(&class_c, {&class_b});

  ClassTreeBuilder builder(&analyze_context());
  builder.Process(&class_b);
  builder.Process(&class_c);
  builder.Process(&class_a);
  builder.Build();

  EXPECT_EQ("", GetErrors());
  EXPECT_EQ(ClassListFrom(class_a), ClassListOf(class_a));
  EXPECT_EQ(ClassListFrom(class_b, class_a), ClassListOf(class_b));
  EXPECT_EQ(ClassListFrom(class_c, class_b, class_a), ClassListOf(class_c));
}

}  // namespace analyzer
}  // namespace joana
