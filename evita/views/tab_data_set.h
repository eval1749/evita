// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_data_model_h)
#define INCLUDE_evita_views_tab_data_model_h

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/dom/windows/window_id.h"

namespace domapi {
struct TabData;
}

namespace views {

class TabDataSet : public common::Singleton<TabDataSet> {
  DECLARE_SINGLETON_CLASS(TabDataSet);

  private: typedef domapi::TabData TabData;

  private: std::unordered_map<int, TabData*> map_;

  private: TabDataSet();
  public: ~TabDataSet();

  public: const TabData* GetTabData(dom::WindowId window_id);
  public: void RemoveTabData(dom::WindowId window_id);
  public: void SetTabData(dom::WindowId window_id, const TabData& tab_data);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_data_model_h)
