// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <unordered_map>

#include "evita/dom/components/win_registry/win_registry.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/win/registry.h"
#include "evita/dom/bindings/exception_state.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// RootKeyMap
//
class RootKeyMap final {
 public:
  RootKeyMap();
  ~RootKeyMap() = default;

  HKEY Get(base::StringPiece16 name) const;
  static RootKeyMap* GetInstance();

 private:
  std::unordered_map<base::StringPiece16, HKEY, base::StringPiece16Hash> map_;

  DISALLOW_COPY_AND_ASSIGN(RootKeyMap);
};

RootKeyMap::RootKeyMap() {
#define INSTALL(name) map_.emplace(L## #name, name)
  INSTALL(HKEY_CLASSES_ROOT);
  INSTALL(HKEY_CURRENT_CONFIG);
  INSTALL(HKEY_CURRENT_USER);
  INSTALL(HKEY_LOCAL_MACHINE);
  INSTALL(HKEY_USERS);
#undef INSTALL
}

HKEY RootKeyMap::Get(base::StringPiece16 name) const {
  const auto& it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

RootKeyMap* RootKeyMap::GetInstance() {
  return base::Singleton<RootKeyMap>::get();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WinRegistry
//
WinRegistry::WinRegistry() {}
WinRegistry::~WinRegistry() {}

// static
base::string16 WinRegistry::ReadString(const base::string16& path,
                                       ExceptionState* exception_state) {
  const auto first_slash = path.find_first_of('\\');
  if (first_slash == base::string16::npos) {
    exception_state->ThrowError(base::StringPrintf(
        "Invalid key %ls", path.substr(0, first_slash).c_str()));
    return base::string16();
  }
  const auto last_slash = path.find_last_of('\\');
  DCHECK_NE(last_slash, base::string16::npos);
  const auto& root_name = path.substr(0, first_slash);
  const auto root_key = RootKeyMap::GetInstance()->Get(root_name);
  if (!root_key) {
    exception_state->ThrowError(
        base::StringPrintf("Invalid root key %ls", root_name.c_str()));
    return base::string16();
  }
  base::win::RegKey reg_key(root_key, path.substr(first_slash).c_str(),
                            KEY_QUERY_VALUE);
  if (!reg_key.Valid())
    return base::string16();
  base::string16 value;
  reg_key.ReadValue(path.substr(last_slash + 1).c_str(), &value);
  return value;
}

}  // namespace dom
