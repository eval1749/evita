// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_FACTORY_SET_H_
#define EVITA_GFX_FACTORY_SET_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/dpi_handler.h"
#include "evita/gfx/size_f.h"

interface IDWriteFactory;
interface IWICImagingFactory;

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FactorySet
//
class FactorySet final : public common::ComInit, public DpiHandler {
 public:
  static ID2D1Factory1& d2d1() { return *instance()->d2d1_factory_; }
  static IDWriteFactory& dwrite() { return *instance()->dwrite_factory_; }
  static IWICImagingFactory& image() { return *instance()->image_factory_; }
  static FactorySet* instance() { return GetInstance(); }

  static SizeF AlignToPixel(const SizeF& size) {
    return instance()->DpiHandler::AlignToPixel(size);
  }
  static SizeF CeilToPixel(const SizeF& size) {
    return instance()->DpiHandler::CeilToPixel(size);
  }

  static FactorySet* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<FactorySet>;

  FactorySet();
  ~FactorySet();

  common::ComPtr<ID2D1Factory1> d2d1_factory_;
  common::ComPtr<IDWriteFactory> dwrite_factory_;
  common::ComPtr<IWICImagingFactory> image_factory_;

  DISALLOW_COPY_AND_ASSIGN(FactorySet);
};

}  // namespace gfx

#endif  // EVITA_GFX_FACTORY_SET_H_
