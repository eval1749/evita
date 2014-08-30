// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_data_model_h)
#define INCLUDE_evita_views_tab_data_model_h

#include <unordered_map>

#include "base/observer_list.h"
#include "common/memory/singleton.h"
#include "evita/dom/windows/window_id.h"

namespace domapi {
struct TabData;
}

namespace views {

class TabDataSet : public common::Singleton<TabDataSet> {
  DECLARE_SINGLETON_CLASS(TabDataSet);

  public: class Observer {
    public: Observer();
    public: virtual ~Observer();

    public: virtual void DidSetTabData(dom::WindowId window_id,
                                       const domapi::TabData& tab_data) = 0;

    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  private: typedef domapi::TabData TabData;

  private: std::unordered_map<int, TabData*> map_;
  private: ObserverList<Observer> observers_;

  private: TabDataSet();
  public: ~TabDataSet();

  public: void AddObserver(Observer* observer);
  public: const TabData* GetTabData(dom::WindowId window_id);
  public: void RemoveObserver(Observer* observer);
  public: void RemoveTabData(dom::WindowId window_id);
  public: void SetTabData(dom::WindowId window_id, const TabData& tab_data);

  DISALLOW_COPY_AND_ASSIGN(TabDataSet);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_data_model_h)
