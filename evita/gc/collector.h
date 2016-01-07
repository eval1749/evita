// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GC_COLLECTOR_H_
#define EVITA_GC_COLLECTOR_H_

#include <memory>
#include <unordered_set>

#include "base/macros.h"
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
