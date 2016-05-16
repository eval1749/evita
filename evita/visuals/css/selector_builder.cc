// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/selector_builder.h"

namespace visuals {
namespace css {

using Builder = Selector::Builder;

Builder::Builder() = default;
Builder::~Builder() = default;

Builder& Builder::AddClass(base::AtomicString class_name) {
  DCHECK(!class_name.empty());
  DCHECK_NE(class_name.value()[0], '.');
  if (classes_.count(class_name) != 0)
    return *this;
  classes_.insert(class_name);
  return *this;
}

Builder& Builder::AddClass(base::StringPiece16 class_name) {
  return AddClass(base::AtomicString(class_name));
}

Selector Builder::Build() {
  Selector selector;
  selector.classes_ = std::move(classes_);
  selector.id_ = id_;
  selector.tag_name_ = tag_name_;
  id_ = base::AtomicString();
  tag_name_ = base::AtomicString();
  return std::move(selector);
}

// style
Selector Builder::AsUniversalSelector(const Selector& selector) {
  Builder builder;
  if (!selector.id().empty())
    builder.SetId(selector.id());
  for (auto class_name : selector.classes())
    builder.AddClass(class_name);
  return std::move(builder.Build());
}

Builder& Builder::SetId(base::AtomicString id) {
  DCHECK(!id.empty());
  DCHECK_NE(id.value()[0], '#');
  DCHECK(id_.empty()) << id_;
  id_ = id;
  return *this;
}

Builder& Builder::SetId(base::StringPiece16 id) {
  return SetId(base::AtomicString(id));
}

Builder& Builder::SetTagName(base::AtomicString tag_name) {
  DCHECK(!tag_name.empty());
  DCHECK(tag_name_.empty()) << tag_name_;
  tag_name_ = tag_name;
  return *this;
}

Builder& Builder::SetTagName(base::StringPiece16 tag_name) {
  return SetTagName(base::AtomicString(tag_name));
}

}  // namespace css
}  // namespace visuals