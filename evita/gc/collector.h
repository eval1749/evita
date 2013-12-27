// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_collector_h)
#define INCLUDE_evita_gc_collector_h

#include <unordered_set>

#include "common/memory/singleton.h"

namespace gc {

class Visitable;

namespace internal {
class AbstractCollectable;
}  // namespace internal

class Collector : public common::Singleton<Collector> {
  public: typedef internal::AbstractCollectable Collectable;
  public: typedef std::unordered_set<Collectable*> CollectableSet;
  public: typedef std::unordered_set<Visitable*> VisitableSet;

  friend class common::Singleton<Collector>;

  private: CollectableSet live_set_;
  private: VisitableSet root_set_;

  private: Collector() = default;
  public: ~Collector() = default;

  public: void AddToRootSet(Visitable* visitable);
  public: void AddToLiveSet(Collectable* collectable);
  public: void CollectGarbage();
  public: void RemoveFromRootSet(Visitable* visitable);

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_collector_h)
