// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/gc/collector.h"

#include "base/logging.h"
#include "evita/gc/collectable.h"
#include "evita/gc/visitor.h"

namespace gc {

namespace {
class CollectorVisitor : public Visitor {
  private: Collector* collector_;
  private: Collector::CollectableSet live_set_;

  public: CollectorVisitor(Collector* collector) : collector_(collector) {
  }
  public: virtual ~CollectorVisitor() = default;

  public: const Collector::CollectableSet& live_set() const {
    return live_set_;
  }

  public: virtual void Visit(Collectable* collectable) override {
    CHECK(!collectable->is_dead());
    collectable->Accept(this);
    live_set_.insert(collectable);
  }

  public: virtual void Visit(Visitable* visitable) override {
    visitable->Accept(this);
  }

  DISALLOW_COPY_AND_ASSIGN(CollectorVisitor);
};
}  // namespace

void Collector::AddToLiveSet(Collectable* collectable) {
  live_set_.insert(collectable);
}

void Collector::AddToRootSet(Visitable* visitable) {
  root_set_.insert(visitable);
}

void Collector::CollectGarbage() {
  CollectorVisitor visitor(this);
  for (auto const visitable: root_set_) {
    visitable->Accept(&visitor);
  }

  CollectableSet live_set;
  for (auto const collectable: visitor.live_set()) {
    live_set_.erase(collectable);
    live_set.insert(collectable);
 }

 for (auto const collectable: live_set_) {
   delete collectable;
 }

 live_set_ = live_set;
}

void Collector::RemoveFromRootSet(Visitable* visitable) {
  auto const count = root_set_.erase(visitable);
  CHECK_EQ(1u, count);
}

}  // namespace gc
