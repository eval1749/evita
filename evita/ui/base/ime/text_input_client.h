// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_base_ime_text_input_client_h)
#define INCLUDE_evita_ui_base_ime_text_input_client_h

#include "base/strings/string16.h"
#include "evita/gfx/rect_f.h"

namespace ui {

class TextInputDelegate;
class Widget;

class TextInputClient {
  private: gfx::RectF caret_bounds_;
  private: TextInputDelegate* delegate_;
  private: bool has_composition_;

  public: TextInputClient();
  public: virtual ~TextInputClient();

  public: TextInputDelegate* delegate() const { return delegate_; }
  public: void set_delegate(TextInputDelegate* new_delegate) {
    delegate_ = new_delegate;
  }
  public: const gfx::RectF& caret_bounds() const { return caret_bounds_; }
  public: void set_caret_bounds(const gfx::RectF& new_bounds);
  public: bool has_composition() const { return has_composition_; }
  protected: void set_has_composition(bool has_composition) {
    has_composition_ = has_composition;
  }

  public: virtual void CancelComposition(TextInputDelegate* requester) = 0;
  public: virtual void CommitComposition(TextInputDelegate* requester) = 0;
  protected: virtual void DidChangeCaretBounds() = 0;
  public: static TextInputClient* Get();
  public: virtual void Reconvert(TextInputDelegate* requester,
                                 const base::string16& text) = 0;

  DISALLOW_COPY_AND_ASSIGN(TextInputClient);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_ime_text_input_client_h)
