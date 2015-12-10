// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_DATA_SET_H_
#define EVITA_VIEWS_TABS_TAB_DATA_SET_H_

#include <unordered_map>

#include "base/observer_list.h"
#include "common/memory/singleton.h"
#include "evita/dom/public/window_id.h"

namespace domapi {
struct TabData;
}

namespace views {

class TabDataSet final : public common::Singleton<TabDataSet> {
  DECLARE_SINGLETON_CLASS(TabDataSet);

 public:
  using TabData = domapi::TabData;

  class Observer {
   public:
    virtual ~Observer();

    virtual void DidSetTabData(domapi::WindowId window_id,
                               const TabData& tab_data) = 0;

   protected:
    Observer();

   private:
    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  ~TabDataSet();

  void AddObserver(Observer* observer);
  const TabData* GetTabData(domapi::WindowId window_id);
  void RemoveObserver(Observer* observer);
  void RemoveTabData(domapi::WindowId window_id);
  void SetTabData(domapi::WindowId window_id, const TabData& tab_data);

 private:
  TabDataSet();

  std::unordered_map<int, TabData*> map_;
  base::ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(TabDataSet);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_DATA_SET_H_
