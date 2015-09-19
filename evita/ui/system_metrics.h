// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_SYSTEM_METRICS_H_
#define EVITA_UI_SYSTEM_METRICS_H_

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/gfx_base.h"

namespace ui {

class SystemMetricsObserver;

class SystemMetrics final : public common::Singleton<SystemMetrics> {
  DECLARE_SINGLETON_CLASS(SystemMetrics);

 public:
  ~SystemMetrics() override;

  gfx::ColorF bgcolor() const { return bgcolor_; }
  gfx::ColorF border_color() const { return border_color_; }
  gfx::ColorF color() const { return color_; }
  const base::string16& font_family() const { return font_family_; }
  gfx::ColorF gray_text() const { return gray_text_; }
  gfx::ColorF highlight() const { return highlight_; }
  gfx::ColorF hotlight() const { return hotlight_; }
  const base::string16& icon_font_family() const { return icon_font_family_; }
  float icon_font_size() const { return icon_font_size_; }
  gfx::ColorF shadow() const { return shadow_; }
  float font_size() const { return font_size_; }

  void AddObserver(SystemMetricsObserver* observer);
  void NotifyChangeIconFont();
  void NotifyChangeSystemColor();
  void NotifyChangeSystemMetrics();
  void RemoveObserver(SystemMetricsObserver* observer);

 private:
  SystemMetrics();

  void UpdateColors();
  void UpdateTextFormat();

  gfx::ColorF bgcolor_;
  gfx::ColorF border_color_;
  gfx::ColorF color_;
  base::string16 font_family_;
  gfx::ColorF gray_text_;
  gfx::ColorF highlight_;
  gfx::ColorF hotlight_;
  base::string16 icon_font_family_;
  float icon_font_size_;
  gfx::ColorF shadow_;
  float font_size_;
  base::ObserverList<SystemMetricsObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(SystemMetrics);
};

}  // namespace ui

#endif  // EVITA_UI_SYSTEM_METRICS_H_
