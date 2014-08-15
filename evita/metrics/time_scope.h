// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_metrics_time_scope_h)
#define INCLUDE_evita_metrics_time_scope_h

#include <memory>
#include <string>
#include <unordered_map>

#include "base/time/time.h"
#include "common/memory/singleton.h"
#include "common/strings/string_piece.h"

namespace metrics {

class Histogram {
  private: std::unordered_map<int, int> map_;
  private: base::StringPiece name_;

  public: Histogram(const base::StringPiece& name);
  public: ~Histogram();

  public: const std::unordered_map<int, int>& data() const {
    return map_;
  }

  public: void AddSample(int value);
};

class HistogramSet : public common::Singleton<HistogramSet> {
  DECLARE_SINGLETON_CLASS(HistogramSet);

  private: std::unordered_map<base::StringPiece, Histogram*> map_;

  private: HistogramSet();
  public: ~HistogramSet();

  public: Histogram* GetOrCreate(const base::StringPiece& name);
  public: base::string16 GetJson(const base::string16& name) const;
};

class TimeScope {
  private: base::StringPiece name_;
  private: base::Time start_at_;
  public: TimeScope(const base::StringPiece& name);
  public: ~TimeScope();

  public: base::TimeDelta value() const;
};

}  // namespace metrics

#define METRICS_TIME_SCOPE() \
    ::metrics::TimeScope metrics_time_scope_ ## __LINE__(__FUNCTION__)

#endif //!defined(INCLUDE_evita_metrics_time_scope_h)
