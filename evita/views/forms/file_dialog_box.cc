// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/file_dialog_box.h"

#pragma warning(push)
// For defining GetOpenFileName and GetSaveFileName.
// warning C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
#pragma warning(disable : 4191)
#include <commdlg.h>
#pragma warning(pop)
#undef GetOpenFileName
#undef GetSaveFileName

#include "evita/editor/modal_message_loop_scope.h"

namespace views {

FileDialogBox::FileDialogBox() {}

FileDialogBox::~FileDialogBox() {}

bool FileDialogBox::GetOpenFileName(Param* pParam) {
  OPENFILENAME oOfn = {0};
  oOfn.lStructSize = sizeof(oOfn);
  oOfn.hwndOwner = pParam->m_hwndOwner;
  // TODO(yosi) We should make filter as parameter to allow scripts specify
  // filters.
  oOfn.lpstrFilter =
      L"All Files\0*.*\0"
      L"C/C++ Files\0*.cpp;*.hpp;*.c;*.h;*.cxx;*.hxx\0"
      L"Lisp Files\0*.lisp;*.l;*.lsp;*.cl\0";
  oOfn.lpstrFile = pParam->m_wsz;
  oOfn.lpstrInitialDir = pParam->m_wszDir[0] ? pParam->m_wszDir : nullptr;

  oOfn.nMaxFile = arraysize(pParam->m_wsz);
  oOfn.Flags = 0;
  oOfn.Flags |= OFN_ENABLESIZING;
  oOfn.Flags |= OFN_EXPLORER;
  oOfn.Flags |= OFN_FILEMUSTEXIST;
  oOfn.Flags |= OFN_PATHMUSTEXIST;

  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  if (!::GetOpenFileNameW(&oOfn))
    return false;

  pParam->m_fReadOnly = (oOfn.Flags & OFN_READONLY) != 0;
  pParam->m_pwszFile = oOfn.lpstrFile + oOfn.nFileOffset;

  return true;
}

bool FileDialogBox::GetSaveFileName(Param* pParam) {
  OPENFILENAME oOfn = {0};
  oOfn.lStructSize = sizeof(oOfn);
  oOfn.hwndOwner = pParam->m_hwndOwner;
  oOfn.lpstrFilter = L"All Files\0*.*\0";
  oOfn.lpstrFile = pParam->m_wsz;
  oOfn.nMaxFile = arraysize(pParam->m_wsz);
  oOfn.lpstrInitialDir = pParam->m_wszDir[0] ? pParam->m_wszDir : nullptr;

  oOfn.Flags = 0;
  oOfn.Flags |= OFN_CREATEPROMPT;
  oOfn.Flags |= OFN_ENABLESIZING;
  oOfn.Flags |= OFN_EXPLORER;
  oOfn.Flags |= OFN_OVERWRITEPROMPT;
  oOfn.Flags |= OFN_PATHMUSTEXIST;
  oOfn.Flags |= OFN_SHAREAWARE;
  editor::ModalMessageLoopScope modal_mesage_loop_scope;
  if (!::GetSaveFileNameW(&oOfn))
    return false;
  pParam->m_pwszFile = oOfn.lpstrFile + oOfn.nFileOffset;
  return true;
}

void FileDialogBox::Param::SetDirectory(const base::char16* pwszFile) {
  if (!*pwszFile)
    return;
  base::char16* pwszFilePart;
  auto const cwchFull =
      ::GetFullPathName(pwszFile, arraysize(m_wszDir), m_wszDir, &pwszFilePart);
  if (cwchFull >= 1 && pwszFilePart)
    *pwszFilePart = 0;
}

}  // namespace views
