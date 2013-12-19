//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileDialogBox.h#1 $
//
#if !defined(INCLUDE_visual_dialog_file_h)
#define INCLUDE_visual_dialog_file_h

// For defining GetOpenFileName and GetSaveFileName.
// warning C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
#pragma warning(disable: 4191)
#include <commdlg.h>
#pragma warning(default: 4191)

//////////////////////////////////////////////////////////////////////
//
// FileDialogBox
//
class FileDialogBox
{
    public: struct Param
    {
        bool    m_fReadOnly;
        HWND    m_hwndOwner;
        char16* m_pwszFile;
        char16  m_wsz[MAX_PATH + 1];
        char16  m_wszDir[MAX_PATH + 1];

        Param()
        {
            myZeroMemory(this, sizeof(*this));
        } // Param

        void SetDirectory(const char16*);
    }; // Param

    // ctro
    public: FileDialogBox();

    // [G]
    public: bool GetOpenFileName(Param*);
    public: bool GetSaveFileName(Param*);
}; // FileDialogBox


#endif //!defined(INCLUDE_visual_dialog_file_h)
