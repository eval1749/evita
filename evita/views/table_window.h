// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABLE_WINDOW_H_
#define EVITA_VIEWS_TABLE_WINDOW_H_

#include <memory>
#include <vector>

#include "evita/views/canvas_content_window.h"

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

class TableWindow final : public CanvasContentWindow,
                          public text::BufferMutationObserver,
                          public ui::TableControlObserver,
                          public ui::TableModel {
  DECLARE_CASTABLE_CLASS(TableView, ContentWindow);

 public:
  TableWindow(WindowId window_id, dom::Document* document);
  ~TableWindow() final;

  std::vector<int> GetRowStates(const std::vector<base::string16>& keys) const;

 private:
  bool DrawIfNeeded();
  void Redraw();
  void Paint();
  std::unique_ptr<TableViewModel> UpdateModelIfNeeded();
  void UpdateControl(std::unique_ptr<TableViewModel> new_model);

  // text::BufferMutationObserver
  void DidDeleteAt(text::Posn offset, size_t length) final;
  void DidInsertAt(text::Posn offset, size_t length) final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::TableControlObserver
  void OnKeyPressed(const ui::KeyEvent&) final;
  void OnMousePressed(const ui::MouseEvent&) final;
  void OnSelectionChanged() final;

  // ui::TableModel
  int GetRowCount() const final;
  int GetRowId(int index) const final;
  base::string16 GetCellText(int row_id, int column_id) const final;

  // views::ContentWindow
  void MakeSelectionVisible() final;

  // ui::Widget
  void DidActivate() final;
  void DidChangeBounds() final;
  void DidSetFocus(ui::Widget* last_focused) final;

  std::vector<ui::TableColumn> columns_;
  ui::TableControl* control_;
  gc::Member<dom::Document> document_;
  std::unique_ptr<TableViewModel> model_;
  std::unordered_map<int, TableViewModel::Row*> row_map_;
  bool should_update_model_;

  DISALLOW_COPY_AND_ASSIGN(TableWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABLE_WINDOW_H_
