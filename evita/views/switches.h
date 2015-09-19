// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_SWITCHES_H_
#define EVITA_VIEWS_SWITCHES_H_

namespace views {
namespace switches {
// All switches in alphabetical order. The switches should be documented
// alongside the definition of their values in the .cc file.extern const char
// kEditorWindowDisplayPaint[];
extern const char kEditorWindowDisplayPaint[];
extern const char kFormWindowDisplayPaint[];
extern const char kTextWindowDisplayPaint[];

extern bool editor_window_display_paint;
extern bool form_window_display_paint;
extern bool text_window_display_paint;

}  // namespace switches
}  // namespace views

#endif  // EVITA_VIEWS_SWITCHES_H_
