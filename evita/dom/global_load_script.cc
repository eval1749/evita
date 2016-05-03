// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

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

bool Global::LoadGlobalScript(ginx::Runner* runner) {
  const auto& raw_list = base::ResourceBundle::GetInstance()->GetRawDatResource(
      GLOBAL_MODULE_JSOBJ);
  if (!raw_list.data()) {
    LOG(FATAL) << "No global js files in resource.";
    return false;
  }
  base::resource::StringPairList string_pair_list(raw_list);
  for (const auto& pair : string_pair_list) {
    const auto& file_name = base::ASCIIToUTF16(pair.first);
    const auto& script_text = base::ASCIIToUTF16(pair.second);
    const auto& result = runner->Run(script_text, file_name);
    if (result.IsEmpty())
      return false;
  }
  return true;
}

}  // namespace dom
