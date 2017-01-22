// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_H_
#define JOANA_ANALYZER_TYPE_H_

#include <iosfwd>
#include <set>
#include <vector>

#include "base/macros.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"

namespace joana {
namespace ast {
class Node;
}
namespace analyzer {

#define DECLARE_ANALYZE_TYPE(name, base) \
  DECLARE_CASTABLE_CLASS(name, base);    \
  friend class Editor;

#define DECLARE_ABSTRACT_ANALYZE_TYPE(name, base) \
  DECLARE_ANALYZE_TYPE(name, base)

#define DECLARE_CONCRETE_ANALYZE_TYPE(name, base) \
  DECLARE_ANALYZE_TYPE(name, base)                \
  friend class TypeFactory;

//
// Type
//
class Type : public Castable<Type>, public ZoneAllocated {
  DECLARE_ABSTRACT_ANALYZE_TYPE(Type, Castable);

 public:
  virtual ~Type();

  // Returns unique identifier of |Type|.
  int id() const { return id_; }

  bool operator==(const Type& other) const;
  bool operator==(const Type* other) const;
  bool operator!=(const Type& other) const;
  bool operator!=(const Type* other) const;

 protected:
  explicit Type(int id);

 private:
  friend class Factory;

  const int id_;

  DISALLOW_COPY_AND_ASSIGN(Type);
};

// See "type_printer.cc" for implementation of |Type| printer.
std::ostream& operator<<(std::ostream& ostream, const Type& type);
std::ostream& operator<<(std::ostream& ostream, const Type* type);
std::ostream& operator<<(std::ostream& ostream,
                         const std::set<const Type*>& types);
std::ostream& operator<<(std::ostream& ostream,
                         const std::vector<const Type*>& types);

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_H_
