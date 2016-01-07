// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_WIN_REGISTRY_H_
#define COMMON_WIN_REGISTRY_H_

#include <windows.h>
#include <winreg.h>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "common/common_export.h"

namespace common {
namespace win {

class COMMON_EXPORT Registry final {
 public:
  Registry(HKEY root, const base::string16& name, REGSAM sam_desired);
  Registry(HKEY root, const base::string16& name);
  ~Registry();

  operator HKEY() const { return hkey_; }

  base::string16 GetValue() const;

 private:
  HKEY hkey_;

  DISALLOW_COPY_AND_ASSIGN(Registry);
};

}  // namespace win
}  // namespace common

#endif  // COMMON_WIN_REGISTRY_H_
