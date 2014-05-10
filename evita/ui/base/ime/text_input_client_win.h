// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_base_ime_text_input_client_win_h)
#define INCLUDE_evita_ui_base_ime_text_input_client_win_h

#include "common/memory/singleton.h"
#include "evita/ui/base/ime/text_input_client.h"

namespace ui {

class TextInputClientWin : public TextInputClient,
                           public common::Singleton<TextInputClientWin> {
  DECLARE_SINGLETON_CLASS(TextInputClientWin);

  private: TextInputClientWin();
  public: ~TextInputClientWin();

  private: void DidUpdateComposition(LPARAM lParam);

  public: std::pair<LRESULT, bool> OnImeMessage(
      uint32_t message, WPARAM wParam, LPARAM lParam);

  public: void Start();

  // ui::TextInputClient
  private: virtual void CancelComposition(
      TextInputDelegate* requester) override;
  private: virtual void CommitComposition(
      TextInputDelegate* requester) override;
  private: virtual void DidChangeCaretBounds() override;
  private: virtual void Reconvert(
      TextInputDelegate* requester, const base::string16& text) override;

  DISALLOW_COPY_AND_ASSIGN(TextInputClientWin);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_ime_text_input_client_win_h)
