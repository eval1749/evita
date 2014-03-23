// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_top_level_widget_observer_h)
#define INCLUDE_evita_ui_top_level_widget_observer_h

namespace ui {

class SystemMetricsObserver {
  protected: SystemMetricsObserver();
  public: virtual ~SystemMetricsObserver();

  public: virtual void DidChangeIconFont();
  public: virtual void DidChangeSystemColor();
  public: virtual void DidChangeSystemMetrics();
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_top_level_widget_observer_h)
