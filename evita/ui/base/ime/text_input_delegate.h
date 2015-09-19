// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_IME_TEXT_INPUT_DELEGATE_H_
#define EVITA_UI_BASE_IME_TEXT_INPUT_DELEGATE_H_

#include "base/macros.h"

namespace ui {

class TextComposition;
class Widget;

class TextInputDelegate {
 public:
  virtual ~TextInputDelegate();

  virtual void DidCommitComposition(const TextComposition& composition) = 0;
  virtual void DidFinishComposition() = 0;
  virtual void DidStartComposition() = 0;
  virtual void DidUpdateComposition(const TextComposition& composition) = 0;
  virtual Widget* GetClientWindow() = 0;

 protected:
  TextInputDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(TextInputDelegate);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_IME_TEXT_INPUT_DELEGATE_H_
