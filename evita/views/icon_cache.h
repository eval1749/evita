// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_ICON_CACHE_H_
#define EVITA_VIEWS_ICON_CACHE_H_

#include <commctrl.h>

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace gfx {
class Bitmap;
class Canvas;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// IconCache
//
class IconCache final {
 public:
  IconCache();
  ~IconCache();

  std::unique_ptr<gfx::Bitmap> BitmapFor(gfx::Canvas* canvas,
                                         int icon_index) const;
  int GetIconForFileName(const base::string16& file_name);

  static IconCache* GetInstance();

 private:
  void Add(const base::string16& name, int icon_index);
  int AddIcon(const base::string16& name, HICON icon);
  int Intern(const base::string16& name);

  std::unordered_map<base::string16, int> map_;
  HIMAGELIST image_list_;

  DISALLOW_COPY_AND_ASSIGN(IconCache);
};

}  // namespace views

#endif  // EVITA_VIEWS_ICON_CACHE_H_
