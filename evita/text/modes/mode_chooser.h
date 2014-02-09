// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_mode_chooser_h)
#define INCLUDE_evita_text_modes_mode_chooser_h

#include <vector>

#include "base/basictypes.h"
#include "common/memory/singleton.h"

namespace text {

class ModeFactory;

class ModeChooser : public common::Singleton<ModeChooser> {
  DECLARE_SINGLETON_CLASS(ModeChooser);

  private: std::vector<ModeFactory*> factories_;
  private: ModeFactory* plain_text_mode_factory_;

  private: ModeChooser();
  public: ~ModeChooser() = default;

  public: ModeFactory* Choose(Buffer* buffer);
};

} // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mode_chooser_h)
