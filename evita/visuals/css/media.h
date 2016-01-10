// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MEDIA_H_
#define EVITA_VISUALS_CSS_MEDIA_H_

#include "base/macros.h"
#include "base/observer_list.h"

namespace visuals {

class FloatSize;

namespace css {

class MediaObserver;
enum class MediaType;

//////////////////////////////////////////////////////////////////////
//
// Media
//
class Media {
 public:
  virtual ~Media();

  virtual MediaType media_type() const = 0;
  virtual FloatSize viewport_size() const = 0;

  void AddObserver(MediaObserver* observer) const;
  void RemoveObserver(MediaObserver* observer) const;

 protected:
  Media();

  void DidChangeViewportSize();
  void DidChangeSystemMetrics();

 private:
  mutable base::ObserverList<MediaObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(Media);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MEDIA_H_
