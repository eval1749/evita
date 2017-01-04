// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PROPERTIES_H_
#define JOANA_ANALYZER_PROPERTIES_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/memory/zone_unordered_map.h"
#include "joana/base/memory/zone_vector.h"

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

//
// Property
//
class Property final : public ZoneAllocated {
 public:
  ~Property();

  const ZoneVector<const ast::Node*>& assignments() const {
    return assignments_;
  }

  const ast::Node& key() const { return key_; }

  const ZoneVector<const ast::Node*>& references() const { return references_; }

 private:
  friend class EnvironmentBuilder;
  friend class Factory;

  Property(Zone* zone, const ast::Node& key);

  void AddAssignment(const ast::Node& assignment);
  void AddReference(const ast::Node& reference);

  ZoneVector<const ast::Node*> assignments_;
  const ast::Node& key_;
  ZoneVector<const ast::Node*> references_;

  DISALLOW_COPY_AND_ASSIGN(Property);
};

//
// Properties
//
class Properties final : public ZoneAllocated {
 public:
  ~Properties();

  // Returns associated property of |key| or null if there is no associated
  // property.
  Property* TryGet(const ast::Node& key) const;

 private:
  friend class Factory;

  Properties(Zone* zone, const ast::Node& node);

  Property& Add(Property* property);

  // TODO(eval1749): We should use another way to handle computed property
  // name.
  ZoneUnorderedMap<base::StringPiece16, Property*, base::StringPiece16Hash>
      computed_name_map_;

  ZoneUnorderedMap<int, Property*> name_map_;

  // AST node which creates this |Properties|.
  const ast::Node& owner_;

  DISALLOW_COPY_AND_ASSIGN(Properties);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PROPERTIES_H_