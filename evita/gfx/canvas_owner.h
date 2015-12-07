// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_CANVAS_OWNER_H_
#define EVITA_GFX_CANVAS_OWNER_H_

#include <memory>

namespace gfx {

class SwapChain;

//////////////////////////////////////////////////////////////////////
//
// CanvasOwner
//
class CanvasOwner {
 public:
  virtual ~CanvasOwner();

  virtual std::unique_ptr<SwapChain> CreateSwapChain() = 0;

 protected:
  CanvasOwner();
};

}  // namespace gfx

#endif  // EVITA_GFX_CANVAS_OWNER_H_
