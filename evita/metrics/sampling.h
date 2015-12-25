// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_METRICS_SAMPLING_H_
#define EVITA_METRICS_SAMPLING_H_

#include <list>

#include "base/macros.h"
#include "base/time/time.h"

namespace metrics {

//////////////////////////////////////////////////////////////////////
//
// Sampling
//
class Sampling final {
 public:
  explicit Sampling(size_t max_samples = 100);
  ~Sampling();

  float last() const { return samples_.back(); }
  float maximum() const { return maximum_; }
  float minimum() const { return minimum_; }
  const std::list<float> samples() const { return samples_; }

  void AddSample(base::TimeDelta sample);
  void AddSample(float sample);

  // Returns value of |GetSystemTimePreciseAsFileTime()| as |base::TimeTicks|.
  static base::TimeTicks NowTimeTicks();

 private:
  float maximum_;
  float minimum_;
  size_t max_samples_;
  std::list<float> samples_;

  DISALLOW_COPY_AND_ASSIGN(Sampling);
};
}  // namespace metrics

#endif  // EVITA_METRICS_SAMPLING_H_
