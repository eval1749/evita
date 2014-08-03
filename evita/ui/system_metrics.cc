// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/system_metrics.h"

#include "common/win/win32_verify.h"
#include "evita/ui/system_metrics_observer.h"

namespace ui {

namespace {
gfx::ColorF GetSysColorF(int index) {
  auto const color_ref = ::GetSysColor(index);
  return gfx::ColorF(GetRValue(color_ref) / 255.0f,
                     GetGValue(color_ref) / 255.0f,
                     GetBValue(color_ref) / 255.0f);
}
}   // namespace

SystemMetrics::SystemMetrics() {
  UpdateColors();
  UpdateTextFormat();
}

SystemMetrics::~SystemMetrics() {
}

void SystemMetrics::AddObserver(SystemMetricsObserver* observer) {
  observers_.AddObserver(observer);
}

void SystemMetrics::NotifyChangeIconFont() {
  UpdateTextFormat();
  FOR_EACH_OBSERVER(SystemMetricsObserver, observers_, DidChangeIconFont());
}

void SystemMetrics::NotifyChangeSystemColor() {
  UpdateColors();
  FOR_EACH_OBSERVER(SystemMetricsObserver, observers_, DidChangeSystemColor());
}

void SystemMetrics::NotifyChangeSystemMetrics() {
  UpdateTextFormat();
  FOR_EACH_OBSERVER(SystemMetricsObserver, observers_,
                    DidChangeSystemMetrics());
}

void SystemMetrics::RemoveObserver(SystemMetricsObserver* observer) {
  observers_.RemoveObserver(observer);
}

void SystemMetrics::UpdateColors() {
  bgcolor_ = GetSysColorF(COLOR_WINDOW);
  border_color_ = GetSysColorF(COLOR_3DDKSHADOW);
  color_ = GetSysColorF(COLOR_WINDOWTEXT);
  gray_text_ = GetSysColorF(COLOR_GRAYTEXT);
  highlight_ = GetSysColorF(COLOR_HIGHLIGHT);
  hotlight_ = GetSysColorF(COLOR_HOTLIGHT);
  shadow_ = GetSysColorF(COLOR_3DDKSHADOW);
}

void SystemMetrics::UpdateTextFormat() {
#if 0
  NONCLIENTMETRICS metrics = {0};
  metrics.cbSize = sizeof(metrics);
  WIN32_VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                                      sizeof(metrics), &metrics, 0));
  auto const log_font = &metrics.lfStatusFont;
  font_family_ = log_font->lfFaceName;
  auto const height_pt = log_font->lfHeight < 0 ? -log_font->lfHeight :
      log_font->lfHeight;
  font_size_ = static_cast<float>(height_pt) * 96.0f / 72.0f;
#else
  font_family_ = L"MS Shell Dlg 2";
  font_size_ = 12.0f;
#endif
}

}  // namespace ui
