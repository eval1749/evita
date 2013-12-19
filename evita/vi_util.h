//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_util.h#1 $
//
#if !defined(INCLUDE_listener_winapp_visual_util_h)
#define INCLUDE_listener_winapp_visual_util_h

// GdiObj
class GdiObj
{
    HGDIOBJ m_h;

    public: GdiObj(HFONT h) :
        m_h(reinterpret_cast<HGDIOBJ>(h)) {}

    public: ~GdiObj()
    {
        if (NULL != m_h) ::DeleteObject(m_h);
    } // ~GdiObj

    public: operator HGDIOBJ() const { return m_h; }

    public: HGDIOBJ Detach()
    {
        HGDIOBJ h = m_h;
        m_h = NULL;
        return h;
    } // Detach
}; // GetiObj


// ScreenDC
class Dc
{
    HWND    m_hwnd;
    HDC     m_hdc;
    public: Dc(HWND hwnd, HDC hdc) : m_hwnd(hwnd), m_hdc(hdc) {}
    public: ~Dc() { if (NULL != m_hdc) ::ReleaseDC(m_hwnd, m_hdc); }
    public: operator HDC() const { return m_hdc; }
}; // Dc


// DcSelect
class DcSelect
{
    HDC     m_hdc;
    HGDIOBJ m_obj;

    public: DcSelect(HDC hdc, HFONT hFont) : m_hdc(hdc)
    {
        m_obj = ::SelectObject(m_hdc, hFont);
    } // DcSelect

    public: DcSelect(HDC hdc, HPEN hPen) : m_hdc(hdc)
    {
        m_obj = ::SelectObject(m_hdc, hPen);
    } // DcSelect

    public: ~DcSelect() { ::SelectObject(m_hdc, m_obj); }
}; // DcSelect


// Pen
class Pen
{
    HPEN m_h;

    public: Pen(Color cr) :
        m_h(::CreatePen(PS_SOLID, 0, cr)) {}

    public: Pen(int iStyle, int iWidth, Color cr) :
        m_h(::CreatePen(iStyle, iWidth, cr)) {}

    public: ~Pen()
        { if (NULL != m_h) ::DeleteObject(m_h); }

    public: operator HPEN() const
        { return m_h; }

    public: operator HGDIOBJ() const
        { return reinterpret_cast<HGDIOBJ>(m_h); }
}; // Pen

#endif //!defined(INCLUDE_listener_winapp_visual_util_h)
