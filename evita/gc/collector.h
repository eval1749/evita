// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_collector_h)
#define INCLUDE_evita_gc_collector_h

#include <memory>
#include <unordered_set>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace base {
class Lock;
}

namespace gc {

class Visitable;

namespace internal {
class AbstractCollectable;
}  // namespace internal

class Collector : public common::Singleton<Collector> {
  DECLARE_SINGLETON_CLASS(Collector);

  public: typedef internal::AbstractCollectable Collectable;
  public: typedef std::unordered_set<Collectable*> CollectableSet;
  public: typedef std::unordered_set<Visitable*> VisitableSet;

  private: CollectableSet live_set_;
  private: std::unique_ptr<base::Lock> lock_;
  private: VisitableSet root_set_;

  private: Collector();
  public: ~Collector();

  public: void AddToRootSet(Visitable* visitable);
  public: void AddToLiveSet(Collectable* collectable);
  public: void CollectGarbage();
  public: base::string16 GetJson(const base::string16& name) const;
  public: void RemoveFromRootSet(Visitable* visitable);

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_collector_h)
