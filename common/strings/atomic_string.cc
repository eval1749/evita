// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/strings/atomic_string.h"

#include <unordered_map>

#include "common/memory/singleton.h"

namespace common {

//////////////////////////////////////////////////////////////////////
//
// AtomicString::Set
//
class AtomicString::Set : public Singleton<AtomicString::Set> {
  DECLARE_SINGLETON_CLASS(Set);

  private: std::unordered_map<base::string16, base::string16*> map_;

  private: Set();
  public: ~Set();

  public: base::string16* GetOrCreate(const base::string16& string);
};

AtomicString::Set::Set() {
}

AtomicString::Set::~Set() {
}

base::string16* AtomicString::Set::GetOrCreate(
    const base::string16& string) {
  auto const it = map_.find(string);
  if (it != map_.end())
    return it->second;
  auto const string_storage = new base::string16(string);
  map_[string] = string_storage;
  return string_storage;
}

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
AtomicString::AtomicString(const base::string16& string)
    : string_(AtomicString::Set::instance()->GetOrCreate(string)) {
}

AtomicString::AtomicString(const AtomicString& other)
    : string_(other.string_) {
}

AtomicString::AtomicString()
    : AtomicString(base::string16()) {
}

AtomicString::~AtomicString() {
}

const AtomicString& AtomicString::Empty() {
  CR_DEFINE_STATIC_LOCAL(AtomicString, empty, (base::string16()));
  return empty;
}

}  // namespace common
