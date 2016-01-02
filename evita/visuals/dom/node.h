// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_NODE_H_
#define EVITA_VISUALS_DOM_NODE_H_

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "common/castable.h"
#include "evita/visuals/css/background.h"
#include "evita/visuals/css/border.h"
#include "evita/visuals/css/float_color.h"
#include "evita/visuals/css/margin.h"
#include "evita/visuals/css/padding.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/dom/nodes_forward.h"

namespace visuals {

class NodeEditor;
class Document;

namespace css {
class Style;
}

#define DECLARE_VISUAL_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)          \
  friend class NodeEditor;

#define DECLARE_VISUAL_NODE_ABSTRACT_CLASS(self, super) \
  DECLARE_VISUAL_NODE_CLASS(self, super)

#define DECLARE_VISUAL_NODE_FINAL_CLASS(self, super) \
  DECLARE_VISUAL_NODE_CLASS(self, super)             \
 private:                                            \
  void Accept(NodeVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public common::Castable {
  DECLARE_VISUAL_NODE_ABSTRACT_CLASS(Node, Castable);

 public:
  class Ancestors;
  class AncestorsOrSelf;
  class Children;
  class Descendants;
  class DescendantsOrSelf;

  virtual ~Node();

  // Tree structure
  Node* next_sibling() const { return next_sibling_; }
  Node* previous_sibling() const { return previous_sibling_; }
  Document* document() const { return document_; }

  // Node identifiers
  const base::string16& id() const { return id_; }
  int sequence_id() const { return sequence_id_; }
  const base::string16& tag_name() const { return tag_name_; }

  // Node tree related values
  ContainerNode* parent() const { return parent_; }

  // CSS background and background
  const css::Background& background() const { return background_; }
  const css::Border& border() const { return border_; }

  // CSS Node model related values
  const css::Margin& margin() const { return margin_; }
  const css::Padding& padding() const { return padding_; }

  // CSS Formatting model
  bool is_display_none() const { return is_display_none_; }

  // CSS Position
  const css::Bottom& bottom() const { return bottom_; }
  const css::Left& left() const { return left_; }
  const css::Position& position() const { return position_; }
  const css::Right& right() const { return right_; }
  const css::Top& top() const { return top_; }

  // CSS Sizing
  const css::Height height() const { return height_; }
  const css::Width width() const { return width_; }

  virtual void Accept(NodeVisitor* visitor) = 0;
  std::unique_ptr<css::Style> ComputeActualStyle() const;

  bool IsDescendantOf(const Node& other) const;

  // Change tracking
  bool IsBackgroundChanged() const { return is_background_changed_; }
  bool IsBorderChanged() const { return is_border_changed_; }
  bool IsContentChanged() const { return is_content_changed_; }
  bool IsOriginChanged() const { return is_origin_changed_; }
  bool IsPaddingChanged() const { return is_padding_changed_; }
  bool IsSizeChanged() const { return is_size_changed_; }

  // Paint
  bool ShouldPaint() const { return should_paint_; }

 protected:
  Node(Document* document,
       const base::StringPiece16& tag_name,
       const base::StringPiece16& id);
  Node(Document* document, const base::StringPiece16& tag_name);

 private:
  // User specified string identifier of this node. Multiple nodes can have
  // same string id.
  const base::string16 id_;
  const int sequence_id_;
  const base::string16 tag_name_;

  bool is_display_none_ = false;
  bool is_background_changed_ = true;
  bool is_border_changed_ = true;
  // When |is_content_changed_| is true, we send bounds of this node to
  // compositor to display on screen. Note: On |ContainerNode|,
  // |is_content_dirty| is always false.
  bool is_content_changed_ = false;
  bool is_origin_changed_ = true;
  bool is_padding_changed_ = true;
  bool is_size_changed_ = true;
  bool should_paint_ = true;

  css::Background background_;
  css::Border border_;
  css::Margin margin_;
  css::Padding padding_;

  // CSS Position
  css::Position position_;
  css::Bottom bottom_;
  css::Left left_;
  css::Right right_;
  css::Top top_;

  // CSS Sizing
  css::Width width_;
  css::Height height_;

  // Tree structure
  Node* next_sibling_ = nullptr;
  Node* previous_sibling_ = nullptr;
  ContainerNode* parent_ = nullptr;
  Document* const document_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

std::ostream& operator<<(std::ostream& ostream, const Node& node);
std::ostream& operator<<(std::ostream& ostream, const Node* node);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_NODE_H_
