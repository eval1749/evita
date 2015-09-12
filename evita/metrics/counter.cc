// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/metrics/counter.h"

#include <sstream>

#include "base/strings/utf_string_conversions.h"
#include "evita/editor/dom_lock.h"

namespace metrics {

Counter::Counter(const base::StringPiece& name) : name_(name) {}

Counter::~Counter() {}

void Counter::AddSample(const base::StringPiece& sample) {
  auto const it = map_.find(sample);
  if (it != map_.end())
    ++it->second;
  else
    map_[sample] = 1;
}

CounterSet::CounterSet() {}

CounterSet::~CounterSet() {}

void CounterSet::AddSample(const base::StringPiece& name,
                           const base::StringPiece& sample) {
  auto const counter = GetOrCreate(name);
  counter->AddSample(sample);
}

Counter* CounterSet::GetOrCreate(const base::StringPiece& name) {
  auto const it = map_.find(name);
  if (it != map_.end())
    return it->second;
  auto const histogram = new Counter(name);
  map_[name] = histogram;
  return histogram;
}

base::string16 CounterSet::GetJson(const base::string16& name) const {
  UI_ASSERT_DOM_LOCKED();
  if (name != L"all")
    return base::string16();
  std::basic_ostringstream<base::char16> ostream;
  ostream << '{';
  const base::string16 comma = L",\n";
  base::string16 delimiter = L"";
  for (auto it : map_) {
    ostream << delimiter << '"' << base::ASCIIToUTF16(it.first) << L"\": [";
    base::string16 delimiter2 = L"";
    for (auto key_value : it.second->data()) {
      ostream << delimiter2;
      ostream << L"{\"key\": \"" << base::ASCIIToUTF16(key_value.first)
              << L"\", \"value\": " << key_value.second << '}';
      delimiter2 = comma;
    }
    ostream << ']';
    delimiter = comma;
  }
  ostream << '}';
  return ostream.str();
}

}  // namespace metrics
