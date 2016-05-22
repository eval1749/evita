// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/property_editor.h"

namespace css {

using Editor = Property::Editor;

//////////////////////////////////////////////////////////////////////
//
// Property::Editor
//
Editor::Editor() = default;
Editor::~Editor() = default;

void Editor::SetValue(Property* property, const Value& value) {
  property->value_ = value;
}

}  // namespace css
