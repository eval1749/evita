// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_metrics_sampling_h)
#define INCLUDE_evita_metrics_sampling_h

#include <list>

#include "base/basictypes.h"
#include "base/time/time.h"

namespace metrics {

//////////////////////////////////////////////////////////////////////
//
// Sampling
//
class Sampling {
  private: float maximum_;
  private: float minimum_;
  private: size_t max_samples_;
  private: std::list<float> samples_;

  public: Sampling(size_t max_samples = 100);
  public: ~Sampling();

  public: float last() const { return samples_.back(); }
  public: float maximum() const { return maximum_; }
  public: float minimum() const { return minimum_; }
  public: const std::list<float> samples() const { return samples_; }

  public: void AddSample(base::TimeDelta sample);
  public: void AddSample(float sample);

  // Returns value of |GetSystemTimePreciseAsFileTime()| as |base::TimeTicks|.
  public: static base::TimeTicks NowTimeTicks();

  DISALLOW_COPY_AND_ASSIGN(Sampling);
};
}  // namespace metrics

#endif //!defined(INCLUDE_evita_metrics_sampling_h)
