// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/bindings.h"

namespace joana {
namespace ast {

//
// ArrayBindingPattern
//
ArrayBindingPattern::ArrayBindingPattern(
    const SourceCodeRange& range,
    const std::vector<const BindingElement*>& elements,
    const Expression& initializer)
    : NodeTemplate(&initializer, range), elements_(elements) {}

ArrayBindingPattern::~ArrayBindingPattern() = default;

//
// BindingCommaElement
//
BindingCommaElement::BindingCommaElement(const SourceCodeRange& range)
    : BindingElement(range) {}

BindingCommaElement::~BindingCommaElement() = default;

//
// BindingElement
//
BindingElement::BindingElement(const SourceCodeRange& range) : Node(range) {}
BindingElement::~BindingElement() = default;

//
// BindingNameElement
//
BindingNameElement::BindingNameElement(const SourceCodeRange& range,
                                       const Name& name,
                                       const Expression& initializer)
    : NodeTemplate(std::make_tuple(&name, &initializer), range) {}

BindingNameElement::~BindingNameElement() = default;

//
// BindingProperty
//
BindingProperty::BindingProperty(const SourceCodeRange& range,
                                 const Name& name,
                                 const BindingElement& element)
    : NodeTemplate(std::make_tuple(&name, &element), range) {}

BindingProperty::~BindingProperty() = default;

//
// BindingRestElement
//
BindingRestElement::BindingRestElement(const SourceCodeRange& range,
                                       const BindingElement& element)
    : NodeTemplate(&element, range) {}

BindingRestElement::~BindingRestElement() = default;

//
// ObjectBindingPattern
//
ObjectBindingPattern::ObjectBindingPattern(
    const SourceCodeRange& range,
    const std::vector<const BindingElement*>& elements,
    const Expression& initializer)
    : NodeTemplate(&initializer, range), elements_(elements) {}

ObjectBindingPattern::~ObjectBindingPattern() = default;

}  // namespace ast
}  // namespace joana
