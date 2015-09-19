// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_FILE_PATH_H_
#define EVITA_DOM_FILE_PATH_H_

#include "evita/v8_glue/scriptable.h"

namespace dom {

class FilePath final : v8_glue::Scriptable<FilePath> {
  DECLARE_SCRIPTABLE_OBJECT(FilePath)

 public:
  FilePath();
  ~FilePath() final;

  static base::string16 FullPath(const base::string16& file_name);

 private:
  DISALLOW_COPY_AND_ASSIGN(FilePath);
};

}  // namespace dom

#endif  // EVITA_DOM_FILE_PATH_H_
