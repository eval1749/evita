// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_VISUALS_NODE_HANDLE_H_
#define EVITA_DOM_VISUALS_NODE_HANDLE_H_

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace gin {
class ArrayBufferView;
}

namespace visuals {
class Node;
}

namespace dom {

namespace bindings {
class NodeHandleClass;
}

class ExceptionState;

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
  int id() const;

  static void AddClass(NodeHandle* element_handle,
                       const base::string16& class_name,
                       ExceptionState* exception_state);
  static void AppendChild(NodeHandle* parent,
                          NodeHandle* node,
                          ExceptionState* exception_state);
  static NodeHandle* CreateDocument();
  static NodeHandle* CreateElement(NodeHandle* document,
                                   const base::string16& tag_name,
                                   const base::string16& id,
                                   ExceptionState* exception_state);
  static NodeHandle* CreateElement(NodeHandle* document,
                                   const base::string16& tag_name,
                                   ExceptionState* exception_state);
  static NodeHandle* CreateShape(NodeHandle* document,
                                 const gin::ArrayBufferView& data,
                                 ExceptionState* exception_state);
  static NodeHandle* CreateText(NodeHandle* document,
                                const base::string16& data,
                                ExceptionState* exception_state);
  static v8::Local<v8::Map> GetInlineStyle(NodeHandle* element,
                                           ExceptionState* exception_state);
  static void InsertBefore(NodeHandle* parent,
                           NodeHandle* node,
                           NodeHandle* child,
                           ExceptionState* exception_state);
  static void RemoveChild(NodeHandle* parent,
                          NodeHandle* node,
                          ExceptionState* exception_state);
  static void RemoveClass(NodeHandle* element_handle,
                          const base::string16& class_name,
                          ExceptionState* exception_state);
  static void ReplaceChild(NodeHandle* parent,
                           NodeHandle* node,
                           NodeHandle* child,
                           ExceptionState* exception_state);
  static void SetInlineStyle(NodeHandle* element,
                             v8::Local<v8::Map> raw_style,
                             ExceptionState* exception_state);
  static void SetShapeData(NodeHandle* element,
                           const gin::ArrayBufferView& data,
                           ExceptionState* exception_state);
  static void SetTextData(NodeHandle* element,
                          const base::string16& data,
                          ExceptionState* exception_state);

  const gc::Member<visuals::Node> value_;

  DISALLOW_COPY_AND_ASSIGN(NodeHandle);
};

}  // namespace dom

#endif  // EVITA_DOM_VISUALS_NODE_HANDLE_H_
