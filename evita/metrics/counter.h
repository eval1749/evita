// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_metrics_counter_h)
#define INCLUDE_evita_metrics_counter_h

#include <unordered_map>

#include "common/memory/singleton.h"
#include "common/strings/string_piece.h"

namespace metrics {

class Counter {
  public: typedef std::unordered_map<base::StringPiece, int> SampleMap;

  private: SampleMap map_;
  private: base::StringPiece name_;

  public: Counter(const base::StringPiece& name);
  public: ~Counter();

  public: const SampleMap& data() const {
    return map_;
  }

  public: void AddSample(const base::StringPiece& sample);
};

class CounterSet : public common::Singleton<CounterSet> {
  DECLARE_SINGLETON_CLASS(CounterSet);

  private: std::unordered_map<base::StringPiece, Counter*> map_;

  private: CounterSet();
  public: ~CounterSet();

  public: void AddSample(const base::StringPiece& name,
                         const base::StringPiece& sample);
  public: Counter* GetOrCreate(const base::StringPiece& name);
  public: base::string16 GetJson(const base::string16& name) const;
};

}  // namespace metrics

#define METRICS_COUNT(sample) \
  ::metrics::CounterSet::instance()->AddSample(__FUNCTION__, sample)

#endif //!defined(INCLUDE_evita_metrics_counter_h)
