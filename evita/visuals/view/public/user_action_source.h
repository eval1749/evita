// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_USER_ACTION_SOURCE_H_
#define EVITA_VISUALS_VIEW_PUBLIC_USER_ACTION_SOURCE_H_

#include "base/macros.h"

namespace visuals {

class Node;

class UserActionSource {
 public:
  class Observer {
   public:
    virtual ~Observer();

    virtual void DidChangeFocusedNode(Node* new_focused_node) = 0;
    virtual void DidChangeHoveredNode(Node* new_hovered_node) = 0;

   protected:
    Observer();
  };

  virtual ~UserActionSource();

  virtual void AddObserver(Observer* observer) const;
  virtual void RemoveObserver(Observer* observer) const;

 protected:
  UserActionSource();
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_USER_ACTION_SOURCE_H_
