// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/document_set.h"

#include <vector>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "evita/dom/converter.h"
#include "evita/dom/document.h"

namespace dom {

namespace {

typedef std::pair<base::string16, base::string16> StringPair;

StringPair SplitByDot(const base::string16& name) {
  const auto last_dot = name.rfind('.');
  if (!last_dot || last_dot == base::string16::npos)
    return StringPair(name, L"");
  return StringPair(name.substr(0, last_dot), name.substr(last_dot));
}

} // namespace

DocumentSet::DocumentSet() {
}

DocumentSet::~DocumentSet() {
}

std::vector<Document*> DocumentSet::list() const {
  std::vector<Document*> list(map_.size());
  list.resize(0);
  for (const auto& pair : map_) {
    list.push_back(pair.second);
  }
  return std::move(list);
}

Document* DocumentSet::Find(const base::string16 name) const {
  auto it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

base::string16 DocumentSet::MakeUniqueName(const base::string16& name) {
  if (!Find(name))
    return name;
  const auto pair = SplitByDot(name);
  auto candidate = name;
  for (auto n = 2; Find(candidate); ++ n) {
    candidate = pair.first + L" (" + base::IntToString16(n) + L")" +
      pair.second;
  }
  return candidate;
}

void DocumentSet::Register(Document* document) {
  CHECK(!Find(document->name()));
  map_[document->name()] = document;
}

void DocumentSet::ResetForTesting() {
  map_.clear();
}

std::vector<Document*> DocumentSet::StaticList() {
  return instance()->list();
}

v8_glue::Nullable<Document> DocumentSet::StaticFind(
    const base::string16& name) {
  return instance()->Find(name);
}

void DocumentSet::StaticRemove(Document* document) {
  instance()->Unregister(document);
}

void DocumentSet::Unregister(Document* document) {
  auto it = map_.find(document->name());
  if (it == map_.end()) {
    // We called |Document.remove()| for |document|.
    return;
  }
  map_.erase(it);
}

}  // namespace dom
