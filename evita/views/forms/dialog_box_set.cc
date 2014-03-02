// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/dialog_box_set.h"

#include "base/logging.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/views/forms/dialog_box.h"

namespace views {

DialogBoxSet::DialogBoxSet() {
}

DialogBoxSet::~DialogBoxSet() {
}

void DialogBoxSet::DidDestroyDomDialog(domapi::DialogBoxId dialog_box_id) {
  ASSERT_CALLED_ON_UI_THREAD();
  DCHECK_NE(domapi::kInvalidDialogBoxId, dialog_box_id);
  auto it = map_.find(dialog_box_id);
  if (it == map_.end()) {
    DVLOG(0) << "Why we don't have a dialog for DialogBoxId " <<
        dialog_box_id << " in DialogBoxIdMap?";
    return;
  }
  map_.erase(it);
}

bool DialogBoxSet::DoIdle(int hint) {
  auto more = false;
  for (auto& it : map_) {
    if (it.second->OnIdle(hint))
      more = true;
  }
  return more;
}

DialogBox* DialogBoxSet::Find(domapi::DialogBoxId dialog_box_id) {
  ASSERT_CALLED_ON_UI_THREAD();
  DCHECK_NE(domapi::kInvalidDialogBoxId, dialog_box_id);
  auto it = map_.find(dialog_box_id);
  return it == map_.end() ? nullptr : it->second;
}

domapi::DialogBoxId DialogBoxSet::Register(DialogBox* dialog) {
  ASSERT_CALLED_ON_UI_THREAD();
  auto const dialog_box_id = dialog->dialog_box_id();
  DCHECK_NE(domapi::kInvalidDialogBoxId, dialog_box_id);
  DCHECK_EQ(0u, map_.count(dialog_box_id));
  map_[dialog_box_id] = dialog;
  return dialog_box_id;
}

void DialogBoxSet::Unregister(domapi::DialogBoxId dialog_box_id) {
  ASSERT_CALLED_ON_UI_THREAD();
  DCHECK_NE(domapi::kInvalidDialogBoxId, dialog_box_id);
  auto const it = map_.find(dialog_box_id);
  if (it != map_.end())
    map_.erase(it);
}

}  // namespace views
