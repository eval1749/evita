// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_base_ime_text_input_delegate_h)
#define INCLUDE_evita_ui_base_ime_text_input_delegate_h

#include "base/macros.h"

namespace ui {

class TextComposition;
class Widget;

class TextInputDelegate  {
  public: TextInputDelegate();
  public: virtual ~TextInputDelegate();

  public: virtual void DidCommitComposition(
      const TextComposition& composition) = 0;
  public: virtual void DidFinishComposition() = 0;
  public: virtual void DidStartComposition() = 0;
  public: virtual void DidUpdateComposition(
      const TextComposition& composition) = 0;
  public: virtual Widget* GetClientWindow() = 0;

  DISALLOW_COPY_AND_ASSIGN(TextInputDelegate);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_ime_text_input_delegate_h)
