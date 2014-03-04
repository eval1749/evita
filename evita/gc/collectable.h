// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_collectable_h)
#define INCLUDE_evita_gc_collectable_h

#include "evita/gc/visitable.h"

namespace gc {

class Collector;
class Visitor;

namespace internal {
class AbstractCollectable : public Visitable {
  friend class Collector;
  friend class Visitor;

  public: enum State {
    kDead,
    kAlive,
  };

  private: State state_;

  protected: AbstractCollectable();
  protected: virtual ~AbstractCollectable();

  public: bool is_dead() const { return state_ == kDead; }
  public: bool is_alive() const { return state_ == kAlive; }

  DISALLOW_COPY_AND_ASSIGN(AbstractCollectable);
};

}  // namespace internal

template<typename T>
class Collectable : public internal::AbstractCollectable {

  protected: Collectable() = default;
  protected: virtual ~Collectable() = default;

  DISALLOW_COPY_AND_ASSIGN(Collectable);
};

}  // namespace gc

#define DECLARE_COLLECTABLE_OBJECT(name) \
  DECLARE_VISITABLE_OBJECT(name)

#define DECLARE_VISITABLE_OBJECT(name) \
  private: virtual const char* visitable_class_name() const override { \
    return #name; \
  } \
  private:

#include <ostream>

std::ostream& operator<<(std::ostream& ostream,
                         const gc::internal::AbstractCollectable& object);

#endif //!defined(INCLUDE_evita_gc_collectable_h)
