// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <unordered_map>

#include "evita/gc/collector.h"

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/lock.h"
#include "evita/gc/collectable.h"
#include "evita/gc/visitor.h"

namespace gc {

namespace {
class CollectorVisitor final : public Visitor {
 public:
  explicit CollectorVisitor(Collector* collector) : collector_(collector) {}
  ~CollectorVisitor() override = default;

  const Collector::CollectableSet& live_set() const { return live_set_; }

  void Visit(Collectable* collectable) override {
    CHECK(!collectable->is_dead());
    if (visited_set_.find(collectable) == visited_set_.end())
      return;
    visited_set_.insert(collectable);
    collectable->Accept(this);
    live_set_.insert(collectable);
  }

  void Visit(Visitable* visitable) override {
    if (visited_set_.find(visitable) == visited_set_.end())
      return;
    visited_set_.insert(visitable);
    visitable->Accept(this);
  }

 private:
  Collector* collector_;
  Collector::CollectableSet live_set_;
  Collector::VisitableSet visited_set_;

  DISALLOW_COPY_AND_ASSIGN(CollectorVisitor);
};

void MapToJson(std::basic_ostringstream<base::char16>& ostream,  // NOLINT
               const base::StringPiece& map_name,
               const std::unordered_map<base::StringPiece, int> map) {
  const base::string16 comma = L",\n";
  base::string16 delimiter = L"";

  ostream << '"' << base::ASCIIToUTF16(map_name) << L"\": [";
  for (auto key_value : map) {
    ostream << delimiter;
    ostream << L"{\"key\": \"" << base::ASCIIToUTF16(key_value.first)
            << L"\", \"value\": " << key_value.second << '}';
    delimiter = comma;
  }
  ostream << ']';
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Collector
//
Collector::Collector() : lock_(new base::Lock()) {}

Collector::~Collector() {}

void Collector::AddToLiveSet(Collectable* collectable) {
  base::AutoLock lock_scope(*lock_);
  live_set_.insert(collectable);
}

void Collector::AddToRootSet(Visitable* visitable) {
  base::AutoLock lock_scope(*lock_);
  root_set_.insert(visitable);
}

void Collector::CollectGarbage() {
  CollectorVisitor visitor(this);
  for (auto const visitable : root_set_) {
    visitable->Accept(&visitor);
  }

  CollectableSet live_set;
  for (auto const collectable : visitor.live_set()) {
    live_set_.erase(collectable);
    live_set.insert(collectable);
  }

  for (auto const collectable : live_set_) {
    delete collectable;
  }

  live_set_ = live_set;
}

base::string16 Collector::GetJson(const base::string16& name) const {
  // TODO(eval1749): Do we really need to have DOM lock here?
  // UI_ASSERT_DOM_LOCKED();
  if (name != L"all")
    return base::string16();

  std::unordered_map<base::StringPiece, int> live_map;
  for (auto const collectable : live_set_) {
    base::StringPiece key(collectable->visitable_class_name());
    auto it = live_map.find(key);
    if (it == live_map.end())
      live_map[key] = 1;
    else
      ++it->second;
  }

  std::unordered_map<base::StringPiece, int> root_map;
  for (auto const visitable : root_set_) {
    base::StringPiece key(visitable->visitable_class_name());
    auto it = root_map.find(key);
    if (it == root_map.end())
      root_map[key] = 1;
    else
      ++it->second;
  }

  std::basic_ostringstream<base::char16> ostream;
  ostream << '{';
  MapToJson(ostream, "live", live_map);
  ostream << L",\n";
  MapToJson(ostream, "root", root_map);
  ostream << '}';
  return ostream.str();
}

void Collector::RemoveFromRootSet(Visitable* visitable) {
  base::AutoLock lock_scope(*lock_);
  auto const count = root_set_.erase(visitable);
  CHECK_EQ(1u, count);
}

}  // namespace gc
