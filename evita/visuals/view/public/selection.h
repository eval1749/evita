// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_VIEW_PUBLIC_SELECTION_H_
#define EVITA_VISUALS_VIEW_PUBLIC_SELECTION_H_

#include "base/observer_list.h"
#include "evita/css/media_observer.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/view/public/selection_model.h"
#include "evita/visuals/view/public/view_lifecycle_client.h"

namespace base {
class RepeatingTimer;
}

namespace css {
class Media;
}

namespace visuals {

class Document;
class Node;
class SelectionEdtior;
class SelectionModel;
class SelectionObserver;
class ViewLifecycle;

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection final : public ViewLifecycleClient,
                        public css::MediaObserver,
                        public DocumentObserver {
 public:
  explicit Selection(ViewLifecycle* lifecycle);
  ~Selection() final;

  const Node& anchor_node() const;
  int anchor_offset() const;
  const Node& focus_node() const;
  int focus_offset() const;

  bool is_caret_on() const { return is_caret_on_; }
  bool is_caret() const;
  bool is_none() const;
  bool is_range() const;

  void AddObserver(SelectionObserver* observer) const;
  void Clear();
  void Collapse(Node* node, int offset);
  void DidPaint();
  void ExtendTo(Node* node, int offset);
  void RemoveObserver(SelectionObserver* observer) const;

 private:
  void DidFireCaretTimer();

  // css::MediaObserver
  void DidChangeMediaState() final;

  // DocumentObserver
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  const std::unique_ptr<base::RepeatingTimer> caret_timer_;
  bool is_caret_on_ = false;
  mutable base::ObserverList<SelectionObserver> observers_;
  const std::unique_ptr<SelectionModel> model_;

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_VIEW_PUBLIC_SELECTION_H_
