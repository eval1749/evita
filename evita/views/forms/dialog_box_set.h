// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_dialog_box_set_h)
#define INCLUDE_evita_views_forms_dialog_box_set_h

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/forms/dialog_box_id.h"

namespace views {

#undef DialogBox
class DialogBox;

//////////////////////////////////////////////////////////////////////
//
// DialogBoxSet
//
// This class represents mapping from widget id to DOM Dialog object.
//
class DialogBoxSet : public common::Singleton<DialogBoxSet> {
  friend class common::Singleton<DialogBoxSet>;

  private: std::unordered_map<dom::DialogBoxId, DialogBox*> map_;

  private: DialogBoxSet();
  public: ~DialogBoxSet();

  public: void DidDestroyDomDialog(dom::DialogBoxId dialog_box_id);
  public: DialogBox* Find(dom::DialogBoxId dialog_box_id);
  public: dom::DialogBoxId Register(DialogBox* dialog);
  public: void Unregister(dom::DialogBoxId dialog_box_id);

  DISALLOW_COPY_AND_ASSIGN(DialogBoxSet);
};

}  // namespace views

#endif //!defined(INCLUDE_visual_dialog_h)
