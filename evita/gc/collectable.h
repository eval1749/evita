// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GC_COLLECTABLE_H_
#define EVITA_GC_COLLECTABLE_H_

#include <ostream>

#include "evita/gc/visitable.h"

namespace gc {

class Collector;
class Visitor;

namespace internal {
class AbstractCollectable : public Visitable {
 public:
  enum State {
    kDead,
    kAlive,
  };

  bool is_dead() const { return state_ == kDead; }
  bool is_alive() const { return state_ == kAlive; }

 protected:
  AbstractCollectable();
  virtual ~AbstractCollectable();

 private:
  friend class Collector;
  friend class Visitor;

  State state_;

  DISALLOW_COPY_AND_ASSIGN(AbstractCollectable);
};

}  // namespace internal

template <typename T>
class Collectable : public internal::AbstractCollectable {
 protected:
  Collectable() = default;
  virtual ~Collectable() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Collectable);
};

}  // namespace gc

#define DECLARE_COLLECTABLE_OBJECT(name) DECLARE_GC_VISITABLE_OBJECT(name)

#define DECLARE_GC_VISITABLE_OBJECT(name) \
 private:                              \
  const char* visitable_class_name() const override { return #name; }

std::ostream& operator<<(std::ostream& ostream,
                         const gc::internal::AbstractCollectable& object);

#endif  // EVITA_GC_COLLECTABLE_H_
