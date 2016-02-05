// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/visuals/node_handle.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/css_style.h"
#include "evita/ginx/runner.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/shape.h"
#include "evita/visuals/dom/shape_data.h"
#include "evita/visuals/dom/text.h"
#include "gin/array_buffer.h"

namespace dom {

using Element = visuals::Element;

namespace {

visuals::ContainerNode* AsContainerNode(NodeHandle* node_handle,
                                        ExceptionState* exception_state) {
  const auto container = node_handle->value()->as<visuals::ContainerNode>();
  if (container)
    return container;
  exception_state->ThrowError("Requires container node");
  return nullptr;
}

visuals::Document* AsDocument(NodeHandle* node_handle,
                              ExceptionState* exception_state) {
  const auto container = node_handle->value()->as<visuals::Document>();
  if (container)
    return container;
  exception_state->ThrowError("Requires document node");
  return nullptr;
}

visuals::Element* AsElement(NodeHandle* node_handle,
                            ExceptionState* exception_state) {
  const auto container = node_handle->value()->as<visuals::Element>();
  if (container)
    return container;
  exception_state->ThrowError("Requires element node");
  return nullptr;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NodeHandle
//
NodeHandle::NodeHandle(visuals::Node* node) : value_(node) {}
NodeHandle::~NodeHandle() {}

int NodeHandle::id() const {
  return value_->sequence_id();
}

// static
void NodeHandle::AddClass(NodeHandle* element_handle,
                          const base::string16& class_name,
                          ExceptionState* exception_state) {
  const auto element = AsElement(element_handle, exception_state);
  if (!element)
    return;
  visuals::NodeEditor().AddClass(element, class_name);
}

// static
NodeHandle* NodeHandle::CreateDocument() {
  return new NodeHandle(new visuals::Document());
}

// static
NodeHandle* NodeHandle::CreateElement(NodeHandle* document_handle,
                                      const base::string16& tag_name,
                                      const base::string16& id,
                                      ExceptionState* exception_state) {
  const auto document = AsDocument(document_handle, exception_state);
  if (!document)
    return nullptr;
  return new NodeHandle(new visuals::Element(document, tag_name, id));
}

// static
NodeHandle* NodeHandle::CreateShape(
    NodeHandle* document_handle,
    const gin::ArrayBufferView& array_buffer_view,
    ExceptionState* exception_state) {
  const auto document = AsDocument(document_handle, exception_state);
  if (!document)
    return nullptr;
  const auto& data = visuals::ShapeData(array_buffer_view.bytes(),
                                        array_buffer_view.num_bytes());
  return new NodeHandle(new visuals::Shape(document, data));
}

// static
NodeHandle* NodeHandle::CreateText(NodeHandle* document_handle,
                                   const base::string16& data,
                                   ExceptionState* exception_state) {
  const auto document = AsDocument(document_handle, exception_state);
  if (!document)
    return nullptr;
  return new NodeHandle(new visuals::Text(document, data));
}

// static
void NodeHandle::AppendChild(NodeHandle* parent,
                             NodeHandle* node,
                             ExceptionState* exception_state) {
  const auto container = AsContainerNode(parent, exception_state);
  if (!container)
    return;
  visuals::NodeEditor().AppendChild(container, node->value());
}

// static
v8::Local<v8::Map> NodeHandle::GetInlineStyle(NodeHandle* element_handle,
                                              ExceptionState* exception_state) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  const auto element = AsElement(element_handle, exception_state);
  if (!element)
    return v8::Local<v8::Map>();
  ginx::Runner::EscapableHandleScope runner_scope(runner);
  const auto& style = element->inline_style();
  if (!style)
    return runner_scope.Escape(v8::Map::New(context->GetIsolate()));
  return runner_scope.Escape(CSSStyle::ConvertToV8(context, *style));
}

// static
void NodeHandle::InsertBefore(NodeHandle* parent,
                              NodeHandle* node,
                              NodeHandle* child,
                              ExceptionState* exception_state) {
  const auto container = AsContainerNode(parent, exception_state);
  if (!container)
    return;
  visuals::NodeEditor().InsertBefore(container, node->value(), child->value());
}

// static
void NodeHandle::RemoveChild(NodeHandle* parent,
                             NodeHandle* child,
                             ExceptionState* exception_state) {
  const auto container = AsContainerNode(parent, exception_state);
  if (!container)
    return;
  visuals::NodeEditor().RemoveChild(container, child->value());
}

// static
void NodeHandle::RemoveClass(NodeHandle* element_handle,
                             const base::string16& class_name,
                             ExceptionState* exception_state) {
  const auto element = AsElement(element_handle, exception_state);
  if (!element)
    return;
  visuals::NodeEditor().RemoveClass(element, class_name);
}

// static
void NodeHandle::ReplaceChild(NodeHandle* parent,
                              NodeHandle* node,
                              NodeHandle* child,
                              ExceptionState* exception_state) {
  const auto container = AsContainerNode(parent, exception_state);
  if (!container)
    return;
  visuals::NodeEditor().ReplaceChild(container, node->value(), child->value());
}

// static
void NodeHandle::SetInlineStyle(NodeHandle* element_handle,
                                v8::Local<v8::Map> raw_style,
                                ExceptionState* exception_state) {
  const auto& runner = ScriptHost::instance()->runner();
  const auto& context = runner->context();
  const auto element = AsElement(element_handle, exception_state);
  if (!element)
    return;
  auto style = CSSStyle::ConvertFromV8(context, raw_style);
  visuals::NodeEditor().SetInlineStyle(element, *style);
}

// static
void NodeHandle::SetShapeData(NodeHandle* shape_handle,
                              const gin::ArrayBufferView& array_buffer_view,
                              ExceptionState* exception_state) {
  const auto shape = shape_handle->value()->as<visuals::Shape>();
  if (!shape) {
    exception_state->ThrowError("Requires shape node");
    return;
  }
  const auto& data = visuals::ShapeData(array_buffer_view.bytes(),
                                        array_buffer_view.num_bytes());
  visuals::NodeEditor().SetShapeData(shape, data);
}

// static
void NodeHandle::SetTextData(NodeHandle* text_handle,
                             const base::string16& data,
                             ExceptionState* exception_state) {
  const auto text = text_handle->value()->as<visuals::Text>();
  if (!text) {
    exception_state->ThrowError("Requires text node");
    return;
  }
  visuals::NodeEditor().SetTextData(text, data);
}

}  // namespace dom
