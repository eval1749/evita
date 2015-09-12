// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FILE_DIALOG_BOX_H_
#define EVITA_VIEWS_FORMS_FILE_DIALOG_BOX_H_

#include <windows.h>

#include "base/strings/string16.h"

namespace views {

class FileDialogBox final {
 public:
  struct Param final {
    bool m_fReadOnly;
    HWND m_hwndOwner;
    base::char16* m_pwszFile;
    base::char16 m_wsz[MAX_PATH + 1];
    base::char16 m_wszDir[MAX_PATH + 1];

    Param() { ::ZeroMemory(this, sizeof(*this)); }

    void SetDirectory(const base::char16*);
  };

  FileDialogBox();
  ~FileDialogBox();

  bool GetOpenFileName(Param* param);
  bool GetSaveFileName(Param* param);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FILE_DIALOG_BOX_H_
