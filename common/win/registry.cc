// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/registry.h"

#include "common/win/win32_verify.h"

namespace common {
namespace win {

namespace {
HKEY OpenKey(HKEY root, const base::string16& name, REGSAM sam_desired) {
  HKEY hkey;
  auto const ret = ::RegOpenKeyEx(root, name.c_str(), 0, sam_desired, &hkey);
  if (ret != ERROR_SUCCESS) {
    WIN32_VERIFY(ret);
    return nullptr;
  }
  return hkey;
}
}  // namespace

Registry::Registry(HKEY root, const base::string16& name, REGSAM sam_desired)
    : hkey_(OpenKey(root, name, sam_desired)) {}

Registry::Registry(HKEY root, const base::string16& name)
    : Registry(root, name, KEY_QUERY_VALUE) {}

Registry::~Registry() {
  if (hkey_) {
    auto const error = ::RegCloseKey(hkey_);
    if (error != ERROR_SUCCESS)
      DVLOG(0) << "RegCloseKey: error=" << error;
  }
}

base::string16 Registry::GetValue() const {
  DWORD byte_size = 0;
  {
    auto const error = ::RegQueryValueEx(hkey_, nullptr, nullptr, nullptr,
                                         nullptr, &byte_size);
    if (error != ERROR_SUCCESS || byte_size <= 2)
      return base::string16();
  }

  base::string16 value(byte_size / sizeof(base::char16), '?');
  auto const error =
      ::RegQueryValueEx(hkey_, nullptr, nullptr, nullptr,
                        reinterpret_cast<BYTE*>(&value[0]), &byte_size);
  if (error != ERROR_SUCCESS)
    return base::string16();
  // Remove trailing zero.
  if (!value.back())
    value.pop_back();
  return std::move(value);
}

}  // namespace win
}  // namespace common
