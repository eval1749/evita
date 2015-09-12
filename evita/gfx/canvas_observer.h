// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_CANVAS_OBSERVER_H_
#define EVITA_GFX_CANVAS_OBSERVER_H_

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// CanvasObserver
//
class CanvasObserver {
 public:
  virtual ~CanvasObserver();

  virtual void DidRecreateCanvas() = 0;

 protected:
  CanvasObserver();
};

}  // namespace gfx

#endif  // EVITA_GFX_CANVAS_OBSERVER_H_
