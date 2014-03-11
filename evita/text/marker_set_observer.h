// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_set_observer_h)
#define INCLUDE_evita_text_marker_set_observer_h

namespace text {

class MarkerSetObserver {
  public: MarkerSetObserver();
  public: virtual ~MarkerSetObserver();

  public: virtual void DidChangeMarker(Posn start, Posn end) = 0;
};

}  // namespace text

#endif // !defined(INCLUDE_evita_text_marker_set_observer_h)
