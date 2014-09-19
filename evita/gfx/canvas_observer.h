// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_canvas_observer_h)
#define INCLUDE_evita_gfx_canvas_observer_h

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasObserver
//
class CanvasObserver {
  protected: CanvasObserver();
  public: virtual ~CanvasObserver();

  public: virtual void DidRecreateCanvas() = 0;
};


} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_canvas_observer_h)
