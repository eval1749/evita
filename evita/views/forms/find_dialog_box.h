// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_find_dialog_box_h)
#define INCLUDE_evita_find_dialog_box_h

#include "evita/views/forms/dialog_box.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
class FindDialogBox final : public DialogBox {
  private: enum Direction {
    kDirectionDown,
    kDirectionUp,
  };

  private: enum ReplaceIn {
    kReplaceInSelection,
    kReplaceInWhole,
  };

  private: Direction direction_;
  private: ReplaceIn replace_in_;

  public: FindDialogBox(DialogBoxId dialog_box_id);
  public: virtual ~FindDialogBox();

  private: void UpdateUI(bool activate = false);

  // views::DialogBox
  private: virtual int GetTemplate() const override;
  private: virtual void onCancel() override;
  private: virtual bool onCommand(WPARAM, LPARAM) override;
  private: virtual bool onInitDialog() override;
  private: virtual void onOk() override;
  private: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM) override;

  DISALLOW_COPY_AND_ASSIGN(FindDialogBox);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_find_dialog_box_h)
