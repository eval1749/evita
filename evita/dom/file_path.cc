// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/file_path.h"

#include "base/strings/string16.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FilePath
//
FilePath::FilePath() {}

FilePath::~FilePath() {}

base::string16 FilePath::FullPath(const base::string16& file_name) {
  base::string16 full_name(MAX_PATH + 1, 0);
  base::char16* file_start = nullptr;
  auto const length = ::GetFullPathNameW(file_name.c_str(),
                                         static_cast<DWORD>(full_name.length()),
                                         &full_name[0], &file_start);
  full_name.resize(length);
  return full_name;
}

}  // namespace dom
