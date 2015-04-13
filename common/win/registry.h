// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_win_registry_h)
#define INCLUDE_common_win_registry_h

#include <windows.h>
#include <winreg.h>

#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {
namespace win {

class COMMON_EXPORT Registry {
  private: HKEY hkey_;

  public: Registry(HKEY root, const base::string16& name, REGSAM sam_desired);
  public: Registry(HKEY root, const base::string16& name);
  public: ~Registry();

  public: operator HKEY() const { return hkey_; }

  public: base::string16 GetValue() const;
};
}  // win
}  // namespace common

#endif //!defined(INCLUDE_common_win_registry_h)
