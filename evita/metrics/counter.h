// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_METRICS_COUNTER_H_
#define EVITA_METRICS_COUNTER_H_

#include <unordered_map>

#include "base/strings/string_piece.h"
#include "common/memory/singleton.h"

namespace metrics {

class Counter final {
 public:
  typedef std::unordered_map<base::StringPiece, int> SampleMap;

  explicit Counter(const base::StringPiece& name);
  ~Counter();

  const SampleMap& data() const { return map_; }

  void AddSample(const base::StringPiece& sample);

 private:
  SampleMap map_;
  base::StringPiece name_;

  DISALLOW_COPY_AND_ASSIGN(Counter);
};

class CounterSet : public common::Singleton<CounterSet> {
  DECLARE_SINGLETON_CLASS(CounterSet);

 public:
  ~CounterSet();

  void AddSample(const base::StringPiece& name,
                 const base::StringPiece& sample);
  Counter* GetOrCreate(const base::StringPiece& name);
  base::string16 GetJson(const base::string16& name) const;

 private:
  CounterSet();

  std::unordered_map<base::StringPiece, Counter*> map_;

  DISALLOW_COPY_AND_ASSIGN(CounterSet);
};

}  // namespace metrics

#define METRICS_COUNT(sample) \
  ::metrics::CounterSet::instance()->AddSample(__FUNCTION__, sample)

#endif  // EVITA_METRICS_COUNTER_H_
