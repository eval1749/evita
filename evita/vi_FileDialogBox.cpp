#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - File Dialog
// listener/winapp/vi_dlg_file.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileDialogBox.cpp#1 $
//
#include "evita/vi_FileDialogBox.h"

#include "evita/editor/dialog_box.h"

// FileDialogBox ctor
FileDialogBox::FileDialogBox()
{
} // FileDialogBox::FileDialogBox


// FileDialogBox::GetOpenFileName
bool FileDialogBox::GetOpenFileName(Param* pParam)
{
    OPENFILENAME oOfn;
    myZeroMemory(&oOfn, sizeof(oOfn));
    oOfn.lStructSize     = sizeof(oOfn);
    oOfn.hwndOwner       = pParam->m_hwndOwner;
    //oOfn.hInstance     = g_hResource;
    oOfn.lpstrFilter     = L"All Files\0*.*\0"
                           L"C/C++ Files\0*.cpp;*.hpp;*.c;*.h;*.cxx;*.hxx\0"
                           L"Lisp Files\0*.lisp;*.l;*.lsp;*.cl\0";
    oOfn.lpstrFile       = pParam->m_wsz;

    oOfn.lpstrInitialDir =
        0 == pParam->m_wszDir[0] ? NULL : pParam->m_wszDir;

    oOfn.nMaxFile        = lengthof(pParam->m_wsz);
    oOfn.Flags           = 0;
    oOfn.Flags           |= OFN_ENABLESIZING;
    oOfn.Flags           |= OFN_EXPLORER;
    oOfn.Flags           |= OFN_FILEMUSTEXIST;
    oOfn.Flags           |= OFN_PATHMUSTEXIST;

    editor::ModalMessageLoopScope modal_mesage_loop_scope;
    if (!::GetOpenFileName(&oOfn))
        return false;

    pParam->m_fReadOnly = (oOfn.Flags & OFN_READONLY) != 0;
    pParam->m_pwszFile = oOfn.lpstrFile + oOfn.nFileOffset;

    return true;
} // FileDialogBox::GetOpenFileName


// FileDialogBox::GetSaveFileNameW
bool FileDialogBox::GetSaveFileNameW(Param* pParam)
{
    OPENFILENAME oOfn;
    myZeroMemory(&oOfn, sizeof(oOfn));
    oOfn.lStructSize = sizeof(oOfn);
    oOfn.hwndOwner   = pParam->m_hwndOwner;
    //oOfn.hInstance   = g_hResource;
    oOfn.lpstrFilter = L"All Files\0*.*\0";
    oOfn.lpstrFile   = pParam->m_wsz;
    oOfn.nMaxFile    = lengthof(pParam->m_wsz);

    oOfn.lpstrInitialDir =
        0 == pParam->m_wszDir[0] ? NULL : pParam->m_wszDir;

    oOfn.Flags       = 0;
    oOfn.Flags       |= OFN_CREATEPROMPT;
    oOfn.Flags       |= OFN_ENABLESIZING;
    oOfn.Flags       |= OFN_EXPLORER;
    oOfn.Flags       |= OFN_OVERWRITEPROMPT;
    oOfn.Flags       |= OFN_PATHMUSTEXIST;
    oOfn.Flags       |= OFN_SHAREAWARE;
    editor::ModalMessageLoopScope modal_mesage_loop_scope;
    if (!::GetSaveFileName(&oOfn))
        return false;

    pParam->m_pwszFile = oOfn.lpstrFile + oOfn.nFileOffset;

    return true;
} // FileDialogBox::GetSaveFileNameW


void
FileDialogBox::Param::SetDirectory(const char16* pwszFile)
{
    if (0 == *pwszFile)
    {
        return;
    }

    char16* pwszFilePart;
    uint cwchFull = ::GetFullPathName(
        pwszFile,
        lengthof(m_wszDir),
        m_wszDir,
        &pwszFilePart );
    if (cwchFull >= 1 && NULL != pwszFilePart)
    {
        *pwszFilePart = 0;
    }
} // FileDialogBox::Param::SetDirectory
