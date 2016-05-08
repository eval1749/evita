// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <algorithm>
#include <iterator>
#include <utility>

#include "evita/dom/global.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/base/resource/resource_bundle.h"
#include "evita/base/resource/string_list.h"
#include "evita/dom/grit/dom_resources.h"
#include "evita/ginx/runner.h"

namespace dom {

namespace {

bool LoadJsBundle(ginx::Runner* runner, base::StringPiece bundle) {
  base::resource::StringPairList string_pair_list(bundle);
  for (const auto& pair : string_pair_list) {
    const auto& file_name = base::ASCIIToUTF16(pair.first);
    const auto& script_text = base::ASCIIToUTF16(pair.second);
    const auto& result = runner->Run(script_text, file_name);
    if (result.IsEmpty())
      return false;
  }
  return true;
}

}  // namespace

bool Global::LoadGlobalScript(ginx::Runner* runner) {
  const auto& bundle = base::ResourceBundle::GetInstance()->GetRawDatResource(
      GLOBAL_MODULE_JSOBJ);
  if (!bundle.data()) {
    LOG(FATAL) << "No global js files in resource.";
    return false;
  }
  return LoadJsBundle(runner, bundle);
}

bool Global::LoadModule(ginx::Runner* runner, base::StringPiece name) {
  const auto& archive =
      base::ResourceBundle::GetInstance()->GetRawDatResource(MODULES_JSLIB);
  if (!archive.data()) {
    LOG(FATAL) << "No javascript library in resource.";
    return false;
  }
  base::resource::StringPairList directory(archive);
  const auto& it = std::lower_bound(
      directory.begin(), directory.end(), name,
      [](const std::pair<base::StringPiece, base::StringPiece>& entry,
         base::StringPiece name) { return entry.first < name; });
  if (it == directory.end() || (*it).first != name)
    return false;
  return LoadJsBundle(runner, (*it).second);
}

}  // namespace dom
