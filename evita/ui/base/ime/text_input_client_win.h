// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_WIN_H_
#define EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_WIN_H_

#include <utility>

#include "common/memory/singleton.h"
#include "evita/ui/base/ime/text_input_client.h"

namespace ui {

class TextInputClientWin final : public TextInputClient,
                                 public common::Singleton<TextInputClientWin> {
  DECLARE_SINGLETON_CLASS(TextInputClientWin);

 public:
  ~TextInputClientWin();

  std::pair<LRESULT, bool> OnImeMessage(uint32_t message,
                                        WPARAM wParam,
                                        LPARAM lParam);

  void Start();

 private:
  TextInputClientWin();
  void DidUpdateComposition(LPARAM lParam);
  HWND GetHwnd() const;

  // ui::TextInputClient
  void CancelComposition(TextInputDelegate* requester) final;
  void CommitComposition(TextInputDelegate* requester) final;
  void DidChangeCaretBounds() final;
  void DidChangeDelegate(TextInputDelegate* old_delegate) final;
  void Reconvert(TextInputDelegate* requester,
                 const base::string16& text) final;

  DISALLOW_COPY_AND_ASSIGN(TextInputClientWin);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_IME_TEXT_INPUT_CLIENT_WIN_H_
