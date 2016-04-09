// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_ICON_CACHE_H_
#define EVITA_VIEWS_ICON_CACHE_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace gfx {
class Bitmap;
class Canvas;
}

namespace visuals {
class ImageBitmap;
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
  int GetIconForFileName(base::StringPiece16 file_name);

  static IconCache* GetInstance();

 private:
  void Add(base::StringPiece16 name, int icon_index);
  int Intern(base::StringPiece16 name);

  std::unordered_map<base::string16, int> map_;
  std::vector<const visuals::ImageBitmap*> bitmaps_;

  DISALLOW_COPY_AND_ASSIGN(IconCache);
};

}  // namespace views

#endif  // EVITA_VIEWS_ICON_CACHE_H_
