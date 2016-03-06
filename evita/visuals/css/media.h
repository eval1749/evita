// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_MEDIA_H_
#define EVITA_VISUALS_CSS_MEDIA_H_

#include "base/macros.h"
#include "base/observer_list.h"

namespace gfx {
class FloatSize;
}

namespace visuals {

namespace css {

class MediaObserver;
enum class MediaState;
enum class MediaType;

//////////////////////////////////////////////////////////////////////
//
// Media
//
class Media {
 public:
  virtual ~Media();

  virtual MediaState media_state() const = 0;
  virtual MediaType media_type() const = 0;
  virtual gfx::FloatSize viewport_size() const = 0;

  // Helper functions for |css::MediaState|.
  bool is_media_active() const;
  bool is_media_inactive() const;
  bool is_media_interactive() const;

  void AddObserver(MediaObserver* observer) const;
  void RemoveObserver(MediaObserver* observer) const;

 protected:
  Media();

  void DidChangeMediaState();
  void DidChangeSystemMetrics();
  void DidChangeViewportSize();

 private:
  mutable base::ObserverList<MediaObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(Media);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_MEDIA_H_
