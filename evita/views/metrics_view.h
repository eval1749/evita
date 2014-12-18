// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_metrics_view_h)
#define INCLUDE_evita_views_metrics_view_h

#include <memory>

#include "base/time/time.h"
#include "evita/ui/widget.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// MetricsView
//
class MetricsView final : public ui::Widget {
  public: class TimingScope final {
    private: base::TimeTicks start_;
    private: MetricsView* view_;

    public: TimingScope(MetricsView* view);
    public: ~TimingScope();

    DISALLOW_COPY_AND_ASSIGN(TimingScope);
  };

  private: class View;

  private: std::unique_ptr<View> view_;

  public: MetricsView();
  public: ~MetricsView() final;

  public: void Animate(base::Time now);
  public: void RecordTime();

  // ui::Widget
  private: void DidRealize() final;

  DISALLOW_COPY_AND_ASSIGN(MetricsView);
};
}  // namespace views

#endif //!defined(INCLUDE_evita_views_metrics_view_h)
