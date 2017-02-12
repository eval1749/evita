// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PROPERTIES_EDITOR_H_
#define JOANA_ANALYZER_PROPERTIES_EDITOR_H_

#include "joana/analyzer/properties.h"

namespace joana {
namespace analyzer {

//
// Properties::Editor
//
class Properties::Editor final {
 public:
  Editor();
  ~Editor();

  void Add(Properties* properties, const Property& property);

 private:
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PROPERTIES_EDITOR_H_
