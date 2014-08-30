// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_system_metrics_h)
#define INCLUDE_evita_ui_system_metrics_h

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/gfx_base.h"

namespace ui {

class SystemMetricsObserver;

class SystemMetrics : public common::Singleton<SystemMetrics> {
  DECLARE_SINGLETON_CLASS(SystemMetrics);

  private: gfx::ColorF bgcolor_;
  private: gfx::ColorF border_color_;
  private: gfx::ColorF color_;
  private: base::string16 font_family_;
  private: gfx::ColorF gray_text_;
  private: gfx::ColorF highlight_;
  private: gfx::ColorF hotlight_;
  private: gfx::ColorF shadow_;
  private: float font_size_;
  private: ObserverList<SystemMetricsObserver> observers_;

  private: SystemMetrics();
  public: ~SystemMetrics();

  public: gfx::ColorF bgcolor() const { return bgcolor_; }
  public: gfx::ColorF border_color() const { return border_color_; }
  public: gfx::ColorF color() const { return color_; }
  public: const base::string16& font_family() const { return font_family_; }
  public: gfx::ColorF gray_text() const { return gray_text_; }
  public: gfx::ColorF highlight() const { return highlight_; }
  public: gfx::ColorF hotlight() const { return hotlight_; }
  public: gfx::ColorF shadow() const { return shadow_; }
  public: float font_size() const { return font_size_; }

  public: void AddObserver(SystemMetricsObserver* observer);
  public: void NotifyChangeIconFont();
  public: void NotifyChangeSystemColor();
  public: void NotifyChangeSystemMetrics();
  public: void RemoveObserver(SystemMetricsObserver* observer);
  private: void UpdateColors();
  private: void UpdateTextFormat();

  DISALLOW_COPY_AND_ASSIGN(SystemMetrics);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_system_metrics_h)
