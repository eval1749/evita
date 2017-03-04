// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_VALUE_EDITOR_H_
#define JOANA_ANALYZER_VALUE_EDITOR_H_

#include <vector>

#include "joana/analyzer/value.h"

namespace joana {
namespace analyzer {

class Class;
class Function;
class ValueHolder;

//
// Value::Editor
//
class Value::Editor final {
 public:
  Editor();
  ~Editor();

  void AddAssignment(const ValueHolder& binding, const Value& value);
  void SetClassHeritage(const Class& class_value,
                        const std::vector<const Class*>& classes);
  void SetClassList(const Class& class_value,
                    const std::vector<const Class*>& classes);

 private:
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_VALUE_EDITOR_H_
