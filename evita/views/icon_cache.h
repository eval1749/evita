// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_view_icon_cache_h)
#define INCLUDE_evita_view_icon_cache_h

#include <commctrl.h>
#include <unordered_map>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"

namespace views {

class IconCache : public common::Singleton<IconCache> {
  DECLARE_SINGLETON_CLASS(IconCache);

  private: std::unordered_map<base::string16, int> map_;
  private: HIMAGELIST image_list_;

  private: IconCache();
  public: ~IconCache();

  public: HIMAGELIST image_list() const { return image_list_; }

  private: void Add(const base::string16& name, int icon_index);
  private: int AddIcon(const base::string16& name, HICON icon);
  public: int GetIconForFileName(const base::string16& file_name) const;
  private: int Intern(const base::string16& name);
};

} // namespace views

#endif //!defined(INCLUDE_evita_view_icon_cache_h)
