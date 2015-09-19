// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_SYSTEM_METRICS_OBSERVER_H_
#define EVITA_UI_SYSTEM_METRICS_OBSERVER_H_

namespace ui {

class SystemMetricsObserver {
 public:
  virtual ~SystemMetricsObserver();

  virtual void DidChangeIconFont();
  virtual void DidChangeSystemColor();
  virtual void DidChangeSystemMetrics();

 protected:
  SystemMetricsObserver();
};

}  // namespace ui

#endif  // EVITA_UI_SYSTEM_METRICS_OBSERVER_H_
