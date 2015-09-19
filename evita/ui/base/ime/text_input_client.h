// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_H_
#define EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_H_

#include "base/strings/string16.h"
#include "evita/gfx/rect_f.h"

namespace ui {

class TextInputDelegate;
class Widget;

class TextInputClient {
 public:
  virtual ~TextInputClient();

  TextInputDelegate* delegate() const { return delegate_; }
  void set_delegate(TextInputDelegate* new_delegate);
  const gfx::RectF& caret_bounds() const { return caret_bounds_; }
  void set_caret_bounds(const gfx::RectF& new_bounds);
  bool has_composition() const { return has_composition_; }

  virtual void CancelComposition(TextInputDelegate* requester) = 0;
  virtual void CommitComposition(TextInputDelegate* requester) = 0;
  static TextInputClient* Get();
  virtual void Reconvert(TextInputDelegate* requester,
                         const base::string16& text) = 0;

 protected:
  TextInputClient();

  void set_has_composition(bool has_composition) {
    has_composition_ = has_composition;
  }

  virtual void DidChangeCaretBounds() = 0;
  virtual void DidChangeDelegate(TextInputDelegate* old_delegate) = 0;

 private:
  gfx::RectF caret_bounds_;
  TextInputDelegate* delegate_;
  bool has_composition_;

  DISALLOW_COPY_AND_ASSIGN(TextInputClient);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_H_
