// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/metrics/sampling.h"

#include <algorithm>

namespace metrics {

Sampling::Sampling(size_t max_samples)
    : max_samples_(max_samples), samples_(max_samples) {
  maximum_ = minimum_ = samples_.front();
}

Sampling::~Sampling() {}

void Sampling::AddSample(base::TimeDelta sample) {
  AddSample(static_cast<float>(sample.InMillisecondsF()));
}

void Sampling::AddSample(float sample) {
  auto const discard_sample = samples_.front();
  samples_.pop_front();
  samples_.push_back(sample);
  maximum_ = std::max(maximum_, sample);
  minimum_ = std::min(minimum_, sample);
  if (discard_sample != maximum_ && discard_sample != minimum_)
    return;
  maximum_ = minimum_ = samples_.front();
  for (auto const runner : samples_) {
    maximum_ = std::max(maximum_, runner);
    minimum_ = std::min(minimum_, runner);
  }
}

base::TimeTicks Sampling::NowTimeTicks() {
#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
  // The number of 100-naooseconds until since the start of January 1, 1601.
  FILETIME ft_now;
  ::GetSystemTimePreciseAsFileTime(&ft_now);
  LARGE_INTEGER now;
  now.HighPart = static_cast<int32_t>(ft_now.dwHighDateTime);
  now.LowPart = ft_now.dwLowDateTime;
  return base::TimeTicks::FromInternalValue(
      now.QuadPart / (base::Time::kNanosecondsPerMicrosecond / 100));
#else
  static LARGE_INTEGER ticks_per_sec;
  if (!ticks_per_sec.QuadPart)
    ::QueryPerformanceFrequency(&ticks_per_sec);
  LARGE_INTEGER counter;
  ::QueryPerformanceCounter(&counter);
  return base::TimeTicks::FromInternalValue(counter.QuadPart *
                                            base::Time::kMicrosecondsPerSecond /
                                            ticks_per_sec.QuadPart);
#endif
}

}  // namespace metrics
