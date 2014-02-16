// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_find_dialog_box_h)
#define INCLUDE_evita_find_dialog_box_h

#include "evita/IStringCursor.h"
#include "evita/resource.h"
#include "evita/vi_DialogBox.h"

class Selection;

namespace text {
class RegexMatcher;
}

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

  // DialogBox
  private: virtual int GetTemplate() const override { return IDD_FIND; }
  private: virtual void onCancel() override;
  private: virtual bool onCommand(WPARAM, LPARAM) override;
  private: virtual bool onInitDialog() override;
  private: virtual void onOk() override;
  private: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM) override;

  DISALLOW_COPY_AND_ASSIGN(FindDialogBox);
};

#endif //!defined(INCLUDE_evita_find_dialog_box_h)
