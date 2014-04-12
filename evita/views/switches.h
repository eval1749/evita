// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_switches_h)
#define INCLUDE_evita_views_switches_h

namespace views{
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

} // switches
}  // namespace views

#endif //!defined(INCLUDE_evita_views_switches_h)
