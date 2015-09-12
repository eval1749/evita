// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GC_COLLECTOR_H_
#define EVITA_GC_COLLECTOR_H_

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

class Collector final : public common::Singleton<Collector> {
  DECLARE_SINGLETON_CLASS(Collector);

 public:
  typedef internal::AbstractCollectable Collectable;
  typedef std::unordered_set<Collectable*> CollectableSet;
  typedef std::unordered_set<Visitable*> VisitableSet;

  ~Collector();

  void AddToRootSet(Visitable* visitable);
  void AddToLiveSet(Collectable* collectable);
  void CollectGarbage();
  base::string16 GetJson(const base::string16& name) const;
  void RemoveFromRootSet(Visitable* visitable);

 private:
  Collector();

  CollectableSet live_set_;
  std::unique_ptr<base::Lock> lock_;
  VisitableSet root_set_;

  DISALLOW_COPY_AND_ASSIGN(Collector);
};

}  // namespace gc

#endif  // EVITA_GC_COLLECTOR_H_
