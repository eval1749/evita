// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_SWITCH_SET_H_
#define EVITA_EDITOR_SWITCH_SET_H_

#include <unordered_map>
#include <vector>

#include "base/strings/string_piece.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/dom/public/switch_value.h"

namespace editor {

class SwitchSet final : public common::Singleton<SwitchSet> {
  DECLARE_SINGLETON_CLASS(SwitchSet);

 public:
  ~SwitchSet();

  std::vector<base::string16> names() const;

  domapi::SwitchValue Get(const base::string16& name) const;
  void Register(const base::StringPiece& name, bool* value_location);
  void Register(const base::StringPiece& name, int* value_location);
  void Set(const base::string16& name, const domapi::SwitchValue& new_value);

 private:
  struct ValueLocation {
    domapi::SwitchValue::Type type;
    void* pointer;
  };

  SwitchSet();

  std::unordered_map<base::string16, ValueLocation> map_;

  DISALLOW_COPY_AND_ASSIGN(SwitchSet);
};

}  // namespace editor

#endif  // EVITA_EDITOR_SWITCH_SET_H_
