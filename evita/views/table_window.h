// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_table_window_h)
#define INCLUDE_evita_views_table_window_h

#include "evita/views/content_window.h"

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "evita/gc/member.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/compositor/layer_owner_delegate.h"
#include "evita/ui/controls/table_control_observer.h"
#include "evita/views/table_view_model.h"
#include "evita/views/window_id.h"

namespace dom {
class Document;
}

namespace ui {
class KeyboardEvent;
class MouseEvent;
class TableControl;
}

namespace views {

class TableWindow final : public text::BufferMutationObserver,
                          public views::ContentWindow,
                          public ui::TableControlObserver,
                          public ui::TableModel {
  DECLARE_CASTABLE_CLASS(TableView, ContentWindow);

  private: std::vector<ui::TableColumn> columns_;
  private: ui::TableControl* control_;
  private: gc::Member<dom::Document> document_;
  private: std::unique_ptr<TableViewModel> model_;
  private: std::unordered_map<int, TableViewModel::Row*> row_map_;
  private: bool should_update_model_;

  public: TableWindow(WindowId window_id, dom::Document* document);
  public: virtual ~TableWindow();

  private: bool DrawIfNeeded();
  public: std::vector<int> GetRowStates(
      const std::vector<base::string16>& keys) const;
  private: void Redraw();
  private: std::unique_ptr<TableViewModel> UpdateModelIfNeeded();
  private: void UpdateControl(std::unique_ptr<TableViewModel> new_model);

  // text::BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  // ui::AnimationFrameHandler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::TableControlObserver
  private: virtual void OnKeyPressed(const ui::KeyboardEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent&) override;
  private: virtual void OnSelectionChanged() override;

  // ui::TableModel
  private: virtual int GetRowCount() const override;
  private: virtual int GetRowId(int index) const override;
  private: virtual base::string16 GetCellText(
      int row_id, int column_id) const override;

  // views::ContentWindow
  private: virtual void MakeSelectionVisible() override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;

  DISALLOW_COPY_AND_ASSIGN(TableWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_table_window_h)
