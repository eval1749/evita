//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_StatusBar.h#1 $
//
#if !defined(INCLUDE_visual_statusBar_h)
#define INCLUDE_visual_statusBar_h

//////////////////////////////////////////////////////////////////////
//
// StatusBar
//
class StatusBar {
  private: int    m_cParts;
  private: HWND   m_hwnd;
  private: RECT   m_rc;
  private: int    m_rgiPart[50];

  public: StatusBar();
  public: ~StatusBar();

  public: operator HWND() const { return m_hwnd; }

  public: int GetCy() const { return m_rc.bottom - m_rc.top; }
  public: bool IsEqual(const int* prgiPart, int cParts) const;
  public: void Realize(HWND hwndParent, int idCtrl);
  public: void SetParts(const int* prgiPart, int cParts);
};

#endif //!defined(INCLUDE_visual_statusBar_h)
