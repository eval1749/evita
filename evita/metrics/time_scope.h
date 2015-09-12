// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_METRICS_TIME_SCOPE_H_
#define EVITA_METRICS_TIME_SCOPE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/strings/string_piece.h"
#include "base/time/time.h"
#include "common/memory/singleton.h"

namespace metrics {

class Histogram final {
 public:
  explicit Histogram(const base::StringPiece& name);
  ~Histogram();

  const std::unordered_map<int, int>& data() const { return map_; }

  void AddSample(int value);

 private:
  std::unordered_map<int, int> map_;
  base::StringPiece name_;

  DISALLOW_COPY_AND_ASSIGN(Histogram);
};

class HistogramSet final : public common::Singleton<HistogramSet> {
  DECLARE_SINGLETON_CLASS(HistogramSet);

 public:
  ~HistogramSet();

  Histogram* GetOrCreate(const base::StringPiece& name);
  base::string16 GetJson(const base::string16& name) const;

 private:
  HistogramSet();

  std::unordered_map<base::StringPiece, Histogram*> map_;

  DISALLOW_COPY_AND_ASSIGN(HistogramSet);
};

class TimeScope final {
 public:
  explicit TimeScope(const base::StringPiece& name);
  ~TimeScope();

  base::TimeDelta value() const;

 private:
  base::StringPiece name_;
  base::Time start_at_;

  DISALLOW_COPY_AND_ASSIGN(TimeScope);
};

}  // namespace metrics

#define METRICS_TIME_SCOPE() \
  ::metrics::TimeScope metrics_time_scope_##__LINE__(__FUNCTION__)

#endif  // EVITA_METRICS_TIME_SCOPE_H_
