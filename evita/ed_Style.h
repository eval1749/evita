//////////////////////////////////////////////////////////////////////////////
//
// Editor - Buffer
// listener/winapp/ed_Buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Style.h#2 $
//
#if !defined(INCLUDE_edit_Style_h)
#define INCLUDE_edit_Style_h

// Color
class Color
{
    COLORREF    m_cr;

    public: Color(COLORREF cr = 0) : m_cr(cr) {}
    public: Color(uint r, uint g, uint b) : m_cr(RGB(r, g, b)) {}

    public: operator COLORREF() const { return m_cr; }

    public: bool Equal(const Color& cr) const { return m_cr == cr.m_cr; }
    public: uint Hash() const { return m_cr; }
}; // Color

enum FontStyle
{
    FontStyle_Normal,

    FontStyle_Italic,
}; // FontStyle


enum FontWeight
{
    FontWeight_Normal,

    FontWeight_Bold,
}; // FontStyle


enum TextDecoration
{
    TextDecoration_None,
    TextDecoration_GreenWave,
    TextDecoration_RedWave,
    TextDecoration_Underline,
    #if SUPPORT_IME
    TextDecoration_ImeInput,        // dotted underline
    TextDecoration_ImeActive,       // 2 pixel underline
    TextDecoration_ImeInactiveA,    // 1 pixel underline
    TextDecoration_ImeInactiveB,    // 1 pixel underline
    #endif // SUPPORT_IME
}; // TextDecoration

struct StyleValues
{
    enum Mask
    {
        Mask_Background = 1 << 0,
        Mask_Color      = 1 << 1,
        Mask_Decoration = 1 << 2,
        Mask_FontFamily = 1 << 3,
        Mask_FontSize   = 1 << 4,
        Mask_FontStyle  = 1 << 5,
        Mask_FontWeight = 1 << 6,
        Mask_Marker     = 1 << 7,
        Mask_Syntax     = 1 << 8,
    }; // Mask

    uint            m_rgfMask;

    #define DefineField(mp_ty, mp_prefix, mp_name) \
        mp_ty m_##mp_prefix##mp_name; \
        mp_ty Get##mp_name() const { return m_##mp_prefix##mp_name; } \
        mp_ty Set##mp_name(mp_ty x) { return m_##mp_prefix##mp_name = x; }

    DefineField(Color,          cr,     Background)
    DefineField(Color,          cr,     Color)
    DefineField(Color,          cr,     Marker)
    DefineField(TextDecoration, e,      Decoration)
    DefineField(const char16*,  pwsz,   FontFamily);
    DefineField(int,            n,      FontSize)
    DefineField(FontStyle,      e,      FontStyle)
    DefineField(FontWeight,     e,      FontWeight)
    DefineField(int,            n,      Syntax);

    #undef DefineField
}; // StyleValues

extern StyleValues g_DefaultStyle;

#endif //!defined(INCLUDE_edit_Style_h)
