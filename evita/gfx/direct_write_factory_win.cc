// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/direct_write_factory_win.h"

#include "base/memory/singleton.h"
#include "common/win/com_verify.h"

#pragma comment(lib, "dwrite.lib")

namespace gfx {

namespace {
base::win::ScopedComPtr<IDWriteFactory> CreateDWriteFactory() {
  base::win::ScopedComPtr<IDWriteFactory> factory;
  COM_VERIFY(
      ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, factory.iid(),
                            reinterpret_cast<IUnknown**>(factory.Receive())));
  return factory;
}
}  // namespace

DirectWriteFactory::DirectWriteFactory() : impl_(CreateDWriteFactory()) {}

DirectWriteFactory::~DirectWriteFactory() {}

// static
DirectWriteFactory* DirectWriteFactory::GetInstance() {
  return base::Singleton<DirectWriteFactory>::get();
}

}  // namespace gfx
