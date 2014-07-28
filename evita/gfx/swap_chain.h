// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_swap_chain_h)
#define INCLUDE_evita_gfx_swap_chain_h

#include "evita/gfx_base.h"

#include <vector>

interface IDXGISwapChain2;

namespace gfx {

class DxDevice;

//////////////////////////////////////////////////////////////////////
//
// SwapChain
//
class SwapChain {
  private: RectF bounds_;
  private: common::ComPtr<ID2D1DeviceContext> d2d_device_context_;
  private: std::vector<Rect> dirty_rects_;
  private: bool is_first_present_;
  private: bool is_ready_;
  private: const common::ComPtr<IDXGISwapChain2> swap_chain_;
  private: HANDLE const swap_chain_waitable_;

  private: SwapChain(common::ComPtr<IDXGISwapChain2> swap_chain);
  public: ~SwapChain();

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: ID2D1DeviceContext* d2d_device_context() const {
    return d2d_device_context_;
  }
  public: IDXGISwapChain2* swap_chain() const { return swap_chain_; }

  public: void AddDirtyRect(const RectF& dirty_rects);
  public: static SwapChain* CreateForComposition(const RectF& bounds);
  public: static SwapChain* CreateForHwnd(HWND hwnd);
  public: void DidChangeBounds(const RectF& new_bounds);
  public: bool IsReady();
  public: void Present();
  private: void UpdateDeviceContext();

  DISALLOW_COPY_AND_ASSIGN(SwapChain);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_swap_chain_h)
