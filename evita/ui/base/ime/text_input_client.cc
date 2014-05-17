// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/base/ime/text_input_client.h"

#include "base/logging.h"

namespace ui {

namespace {
TextInputClient* static_instance;
}

TextInputClient::TextInputClient() : delegate_(nullptr) {
  DCHECK(!static_instance);
  static_instance = this;
}

TextInputClient::~TextInputClient() {
}

void TextInputClient::set_caret_bounds(const gfx::RectF& new_bounds) {
  if (caret_bounds_ == new_bounds)
    return;
  caret_bounds_ = new_bounds;
  DidChangeCaretBounds();
}

void TextInputClient::set_delegate(TextInputDelegate* new_delegate) {
  if (delegate_ == new_delegate)
    return;
  auto const old_delegate = delegate_;
  delegate_ = new_delegate;
  DidChangeDelegate(old_delegate);
}

// static
TextInputClient* TextInputClient::Get() {
  return static_instance;
}

}  // namespace ui
