// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_view_table_view_h)
#define INCLUDE_view_table_view_h

#include "evita/content/content_window.h"

#include <memory>

#include "base/basictypes.h"
#include "evita/gc/member.h"

namespace dom {
class Document;
}

namespace view {

class TableModel;

class TableView : public CommandWindow_<TableView, content::ContentWindow> {
  DECLARE_CASTABLE_CLASS(TableView, content::ContentWindow);

  private: content::ContentWindow BaseWindow;

  private: gc::Member<dom::Document> document_;
  private: HWND list_view_;
  private: std::unique_ptr<TableModel> model_;

  public: TableView();
  public: virtual ~TableMode();

  private: void Redraw();

  // widgets::Widget
  private: virtual void Hide() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: virtual void Show() override;
  private: virtual void WillDestroyWidget() override;

  DISALLOW_COPY_AND_ASSIGN(TableView);
};

}  // namespace view

#endif //!defined(INCLUDE_view_table_view_h)
