// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/strings/atomic_string_factory.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "evita/base/memory/zone.h"
#include "evita/base/strings/atomic_string.h"

namespace evita {

//////////////////////////////////////////////////////////////////////
//
// AtomicStringFactory
//
AtomicStringFactory::AtomicStringFactory() : ZoneOwner("AtomicStringFactory") {}

AtomicStringFactory::~AtomicStringFactory() {}

AtomicString AtomicStringFactory::New(base::StringPiece16 value) {
  const auto& it = map_.find(value);
  if (it != map_.end())
    return AtomicString(it->second);
  const auto& string_piece = NewStringPiece(value);
  const auto& result = map_.emplace(*string_piece, string_piece);
  DCHECK(result.second) << "Must be inserted";
  return AtomicString(value);
}

base::StringPiece16* AtomicStringFactory::NewStringPiece(
    base::StringPiece16 string_piece) {
  const auto size = string_piece.size() * sizeof(base::char16);
  const auto string = static_cast<base::char16*>(Allocate(size));
  ::memcpy(string, string_piece.data(), size);
  return new (Allocate(sizeof(base::StringPiece16)))
      base::StringPiece16(string, string_piece.size());
}

AtomicString AtomicStringFactory::NewUniqueString(const base::char16* format) {
  for (;;) {
    const auto string = base::StringPrintf(format, ++unique_name_counter_);
    const auto it = map_.find(string);
    if (it == map_.end())
      return New(string);
  }
}

// static
AtomicStringFactory* AtomicStringFactory::GetInstance() {
  return base::Singleton<AtomicStringFactory>::get();
}

}  // namespace evita
