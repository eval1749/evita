// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_SCROLL_BAR_DATA_H_
#define EVITA_DOM_PUBLIC_SCROLL_BAR_DATA_H_

#include "evita/base/float_range.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// ScrollBarData
//
class ScrollBarData final {
 public:
  ScrollBarData(const base::FloatRange& track, const base::FloatRange& thumb);
  ScrollBarData(const ScrollBarData& other);
  ScrollBarData();
  ~ScrollBarData();

  ScrollBarData& operator=(const ScrollBarData& other);

  bool operator==(const ScrollBarData& other) const;
  bool operator!=(const ScrollBarData& other) const;

  const base::FloatRange& thumb() const { return thumb_; }
  const base::FloatRange& track() const { return track_; }

 private:
  base::FloatRange thumb_;
  base::FloatRange track_;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_SCROLL_BAR_DATA_H_
