// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_find_dialog_box_h)
#define INCLUDE_evita_find_dialog_box_h

#include "./IStringCursor.h"
#include "./resource.h"
#include "./vi_DialogBox.h"

class RegexMatcher;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
class FindDialogBox final : public DialogBox {
  public: enum Direction {
    Direction_Down,
    Direction_Up,
  };

  public: enum ReplaceIn {
    ReplaceIn_Selection,
    ReplaceIn_Whole,
  };

  private: Direction m_eDirection;
  private: ReplaceIn m_eReplaceIn;

  public: FindDialogBox();
  public: virtual ~FindDialogBox();

  private: void clearMessage();

  public: void DoFind(Direction);
  private: void doReplace(uint);
  private: bool findFirst(RegexMatcher*);
  private: virtual int GetTemplate() const override { return IDD_FIND; }
  private: void onCancel();
  private: virtual bool onCommand(WPARAM, LPARAM) override;
  private: virtual bool onInitDialog() override;
  private: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM);
  private: void onFindNext();
  private: void onFindPrevious();
  private: void onOk();
  private: void onReplaceOne();
  private: void onReplaceAll();
  private: Selection* prepareFind(SearchParameters*);
  private: void reportNotFound();
  private: void updateUI(bool activate = false);
};

#endif //!defined(INCLUDE_evita_find_dialog_box_h)
