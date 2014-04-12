// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_editor_switch_set_h)
#define INCLUDE_evita_editor_switch_set_h

#include <unordered_map>
#include <vector>

#include "base/strings/string_piece.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/dom/public/switch_value.h"

namespace editor {

class SwitchSet : public common::Singleton<SwitchSet> {
  DECLARE_SINGLETON_CLASS(SwitchSet);

  private: struct ValueLocation {
    domapi::SwitchValue::Type type;
    void* pointer;
  };

  private: std::unordered_map<base::string16, ValueLocation> map_;

  private: SwitchSet();
  public: ~SwitchSet();

  public: std::vector<base::string16> names() const;

  public: domapi::SwitchValue Get(const base::string16& name) const;
  public: void Register(const base::StringPiece& name, bool* value_location);
  public: void Register(const base::StringPiece& name, int* value_location);
  public: void Set(const base::string16& name,
                   const domapi::SwitchValue& new_value);

  DISALLOW_COPY_AND_ASSIGN(SwitchSet);
};

}  // namespace editor

#endif //!defined(INCLUDE_evita_editor_switch_set_h)
