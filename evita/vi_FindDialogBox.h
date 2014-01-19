// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_find_dialog_box_h)
#define INCLUDE_evita_find_dialog_box_h

#include "evita/IStringCursor.h"
#include "evita/resource.h"
#include "evita/text/search_and_replace_model.h"
#include "evita/vi_DialogBox.h"

class RegexMatcher;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// FindDialogBox
//
class FindDialogBox final : public DialogBox {
  private: text::Direction direction_;
  private: text::ReplaceIn replace_in_;

  public: FindDialogBox();
  public: virtual ~FindDialogBox();

  private: void ClearMessage();
  public: void DoFind(text::Direction);
  private: void DoReplace(text::ReplaceMode replace_mode);
  private: bool FindFirst(RegexMatcher* matcher);
  private: void onFindNext();
  private: void onFindPrevious();
  private: void onReplaceOne();
  private: void onReplaceAll();
  private: Selection* PrepareFind(SearchParameters*);
  private: void ReportNotFound();
  private: void UpdateUI(bool activate = false);

  // DialogBox
  private: virtual int GetTemplate() const override { return IDD_FIND; }
  private: virtual bool onInitDialog() override;
  private: virtual void onCancel() override;
  private: virtual bool onCommand(WPARAM, LPARAM) override;
  private: virtual INT_PTR onMessage(UINT, WPARAM, LPARAM) override;
  private: virtual void onOk() override;
};

#endif //!defined(INCLUDE_evita_find_dialog_box_h)
