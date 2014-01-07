// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_file_path_h)
#define INCLUDE_evita_dom_file_path_h

#include "evita/v8_glue/scriptable.h"

namespace dom {

class FilePath : v8_glue::Scriptable<FilePath> {
  public: FilePath();
  public: virtual ~FilePath();

  public: static v8_glue::WrapperInfo* static_wrapper_info();

  DISALLOW_COPY_AND_ASSIGN(FilePath);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_file_path_h)
