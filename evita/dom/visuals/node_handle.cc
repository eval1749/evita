// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/node_handle.h"

#include "evita/dom/script_host.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/text.h"

namespace dom {

visuals::ContainerNode* AsContainerNode(NodeHandle* node) {
  const auto container = node->value()->as<visuals::ContainerNode>();
  if (container)
    return container;
  ScriptHost::instance()->ThrowError("Requires container node");
  return nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// NodeHandle
//
NodeHandle::NodeHandle(visuals::Node* node) : value_(node) {}
NodeHandle::~NodeHandle() {}

int NodeHandle::id() const {
  return value_->sequence_id();
}

void NodeHandle::AddClass(NodeHandle* element_handle,
                          const base::string16& class_name) {
  const auto element = element_handle->value()->as<visuals::Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element node");
    return;
  }
  visuals::NodeEditor().AddClass(element, class_name);
}

NodeHandle* NodeHandle::CreateDocument() {
  return new NodeHandle(new visuals::Document());
}

NodeHandle* NodeHandle::CreateElement(NodeHandle* document_node,
                                      const base::string16& tag_name,
                                      const base::string16& id) {
  const auto document = document_node->value()->as<visuals::Document>();
  if (!document) {
    ScriptHost::instance()->ThrowError("Requires document object");
    return nullptr;
  }
  return new NodeHandle(new visuals::Element(document, tag_name, id));
}

NodeHandle* NodeHandle::CreateText(NodeHandle* document_node,
                                   const base::string16& data) {
  const auto document = document_node->value()->as<visuals::Document>();
  if (!document) {
    ScriptHost::instance()->ThrowError("Requires document node");
    return nullptr;
  }
  return new NodeHandle(new visuals::Text(document, data));
}

void NodeHandle::AppendChild(NodeHandle* parent, NodeHandle* node) {
  const auto container = AsContainerNode(parent);
  if (!container)
    return;
  visuals::NodeEditor().AppendChild(container, node->value());
}

void NodeHandle::InsertBefore(NodeHandle* parent,
                              NodeHandle* node,
                              NodeHandle* child) {
  const auto container = AsContainerNode(parent);
  if (!container)
    return;
  visuals::NodeEditor().InsertBefore(container, node->value(), child->value());
}

void NodeHandle::RemoveChild(NodeHandle* parent, NodeHandle* child) {
  const auto container = AsContainerNode(parent);
  if (!container)
    return;
  visuals::NodeEditor().RemoveChild(container, child->value());
}

void NodeHandle::RemoveClass(NodeHandle* element_handle,
                             const base::string16& class_name) {
  const auto element = element_handle->value()->as<visuals::Element>();
  if (!element) {
    ScriptHost::instance()->ThrowError("Requires element node");
    return;
  }
  visuals::NodeEditor().RemoveClass(element, class_name);
}

}  // namespace dom
