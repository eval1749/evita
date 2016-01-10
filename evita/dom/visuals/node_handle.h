// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_VISUALS_NODE_HANDLE_H_
#define EVITA_DOM_VISUALS_NODE_HANDLE_H_

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace visuals {
class Node;
}

namespace dom {

namespace bindings {
class NodeHandleClass;
}

//////////////////////////////////////////////////////////////////////
//
// NodeHandle
//
class NodeHandle final : public v8_glue::Scriptable<NodeHandle> {
  DECLARE_SCRIPTABLE_OBJECT(NodeHandle);

 public:
  ~NodeHandle() final;

  visuals::Node* value() const { return value_; }

 private:
  friend class bindings::NodeHandleClass;

  explicit NodeHandle(visuals::Node* node);

  // Bindings implementations
  static void AppendChild(NodeHandle* parent, NodeHandle* node);
  static NodeHandle* CreateDocument();
  static NodeHandle* CreateElement(NodeHandle* document,
                                   const base::string16& tag_name,
                                   const base::string16& id);
  static NodeHandle* CreateElement(NodeHandle* document,
                                   const base::string16& tag_name);
  static NodeHandle* CreateText(NodeHandle* document,
                                const base::string16& data);
  static v8::Local<v8::Map> GetInlineStyle(NodeHandle* element);
  static void InsertBefore(NodeHandle* parent,
                           NodeHandle* node,
                           NodeHandle* child);
  static void RemoveChild(NodeHandle* parent, NodeHandle* node);
  static void SetInlineStyle(NodeHandle* element, v8::Local<v8::Map> raw_style);

  const gc::Member<visuals::Node> value_;

  DISALLOW_COPY_AND_ASSIGN(NodeHandle);
};

}  // namespace dom

#endif  // EVITA_DOM_VISUALS_NODE_HANDLE_H_
