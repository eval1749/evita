// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_file_dialog_box_h)
#define INCLUDE_evita_views_forms_file_dialog_box_h

namespace views {

class FileDialogBox {
  public: struct Param {
    bool    m_fReadOnly;
    HWND    m_hwndOwner;
    char16* m_pwszFile;
    char16  m_wsz[MAX_PATH + 1];
    char16  m_wszDir[MAX_PATH + 1];

    Param() {
      ::ZeroMemory(this, sizeof(*this));
    }

    void SetDirectory(const char16*);
  };

  public: FileDialogBox();
  public: ~FileDialogBox();

  public: bool GetOpenFileName(Param*);
  public: bool GetSaveFileName(Param*);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_forms_file_dialog_box_h)
