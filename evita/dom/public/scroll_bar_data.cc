// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/scroll_bar_data.h"

#include "base/logging.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// ScrollBarData
//
ScrollBarData::ScrollBarData(const base::FloatRange& track_range,
                             const base::FloatRange& thumb_range)
    : thumb_(thumb_range), track_(track_range) {
  DCHECK_GE(thumb_.lower(), track_.lower());
  DCHECK_LE(thumb_.upper(), track_.upper());
}

ScrollBarData::ScrollBarData(const ScrollBarData& other)
    : ScrollBarData(other.track_, other.thumb_) {}

ScrollBarData::ScrollBarData() {}
ScrollBarData::~ScrollBarData() {}

ScrollBarData& ScrollBarData::operator=(const ScrollBarData& other) {
  thumb_ = other.thumb_;
  track_ = other.track_;
  return *this;
}

bool ScrollBarData::operator==(const ScrollBarData& other) const {
  return thumb_ == other.thumb_ && track_ == other.track_;
}

bool ScrollBarData::operator!=(const ScrollBarData& other) const {
  return !operator==(other);
}

}  // namespace domapi
