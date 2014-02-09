// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_cxx_mode_h)
#define INCLUDE_evita_text_modes_cxx_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode_factory.h"

namespace text {

class CxxModeFactory : public common::Singleton<CxxModeFactory>,
                       public ModeFactory {
  DECLARE_SINGLETON_CLASS(CxxModeFactory);

  private: CxxModeFactory();
  public: ~CxxModeFactory();

  public: virtual Mode* Create() override;
  protected: virtual const char16* getExtensions() const override {
    return L"cc cpp cxx c hpp hxx h css cs ev mm";
  }
  public: virtual const char16* GetName() const override { return L"C++"; }

  DISALLOW_COPY_AND_ASSIGN(CxxModeFactory);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_cxx_mode_h)
