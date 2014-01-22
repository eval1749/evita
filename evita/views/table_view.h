// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_table_view_h)
#define INCLUDE_evita_views_table_view_h

#include "evita/views/content_window.h"

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "evita/gc/member.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/controls/table_control_observer.h"
#include "evita/views/table_view_model.h"
#include "evita/views/window_id.h"

namespace dom {
class Document;
}

namespace ui {
class TableControl;
}

namespace views {

class TableView
    : public CommandWindow_<TableView, views::ContentWindow>,
      public ui::TableControlObserver,
      public ui::TableModel {
  DECLARE_CASTABLE_CLASS(TableView, views::ContentWindow);

  private: typedef views::ContentWindow BaseWindow;

  private: std::vector<ui::TableColumn> columns_;
  private: std::vector<TableViewModel::Row*> rows_;
  private: std::unordered_map<int, TableViewModel::Row*> row_map_;
  private: ui::TableControl* control_;
  private: gc::Member<dom::Document> document_;
  private: std::unique_ptr<TableViewModel> model_;
  private: int modified_tick_;

  public: TableView(WindowId window_id, dom::Document* document);
  public: virtual ~TableView();

  private: std::unique_ptr<TableViewModel> CreateModel();
  private: bool DrawIfNeeded();
  public: void GetRowStates(const std::vector<base::string16>& keys,
                            int* states) const;

  // ui::TableControlObserver
  private: virtual void OnKeyDown(int key_code) override;
  private: virtual void OnSelectionChanged() override;

  // ui::TableModel
  private: virtual int GetRowCount() const override;
  private: virtual int GetRowId(int index) const override;
  private: virtual base::string16 GetCellText(
      int row_id, int column_id) const override;

  // views::CommandWindow
  private: virtual Command::KeyBindEntry* MapKey(uint) override;

  // views::ContentWindow
  private: virtual base::string16 GetTitle(size_t max_length) const;
  private: virtual void MakeSelectionVisible() override;
  private: virtual void Redraw() override;
  private: virtual void UpdateStatusBar() const override;

  // widgets::Widget
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: virtual bool OnIdle(uint32 idle_count) override;
  private: virtual void Show() override;

  DISALLOW_COPY_AND_ASSIGN(TableView);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_table_view_h)
