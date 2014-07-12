//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Style.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_fontset_h)
#define INCLUDE_listener_winapp_visual_fontset_h

#include <memory>

#include "base/strings/string16.h"
#include "gfx/forward.h"
#include "evita/li_util.h"

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// Font class
//
class Font {
    public: typedef LOGFONT Key;

    private: class FontImpl;
    private: struct SimpleMetrics {
      float ascent;
      float descent;
      float height;
      float fixed_width;
    };

    private: LOGFONT m_oLogFont;
    private: const std::unique_ptr<FontImpl> font_impl_;
    private: const SimpleMetrics metrics_;

    public: Font(const LOGFONT* logFont);
    public: ~Font();

    private: float ascent() const { return metrics_.ascent; }
    public: float descent() const { return metrics_.descent; }
    public: float height() const { return metrics_.height; }

    // [D]
    public: void DrawText(const gfx::Canvas& gfx,const gfx::Brush& text_brush,
                  const gfx::RectF& rect, const base::char16* chars,
                  uint num_chars) const;
    public: void DrawText(const gfx::Canvas& gfx,const gfx::Brush& text_brush,
                  const gfx::RectF& rect, const base::string16& string) const;

    // [E]
    public: bool EqualKey(const Key* pKey) const {
      return !::memcmp(&m_oLogFont, pKey, sizeof(m_oLogFont));
    }

    // [G]
    public: float GetCharWidth(char16) const;
    public: const Key* GetKey() const { return &m_oLogFont; }
    public: float GetTextWidth(const base::char16* pwch, size_t cwch) const;
    public: float GetTextWidth(const base::string16& string) const;

    // [H]
    public: bool HasCharacter(char16) const;

    public: uint Hash() const {
      return static_cast<uint>(reinterpret_cast<UINT_PTR>(this));
    }

    public: static int HashKey(const Key*);

    DISALLOW_COPY_AND_ASSIGN(Font);
}; // Font


//////////////////////////////////////////////////////////////////////
//
// FontSet class
//
struct Fonts
{
    Font*   m_rgpFont[10];
    int     m_cFonts;
}; // Fonts

class FontSet : public Fonts
{
    public: typedef Fonts Key;

    public: FontSet()
    {
        m_cFonts = 0;
    } // FontSet

    // [A]
    public: void Add(Font*);

    // [E]
    public: bool EqualKey(const Key* pFonts) const;

    // [F]
    public: Font* FindFont(const gfx::Canvas&, char16 wch) const {
      return FindFont(wch);
    }

    public: Font* FindFont(char16) const;

    // [G]
    public: static FontSet* Get(const gfx::Canvas&,
                                const css::Style& style) {
      return Get(style);
    }
    public: static FontSet* Get(const css::Style& style);
    public: const Key* GetKey() const { return this; }

    // [H]
    public: static int HashKey(const Key*);

    // [E]
    public: class EnumFont
    {
        Font**  m_pRunner;
        Font**  m_pEnd;

        public: EnumFont(const FontSet* p) :
            m_pRunner(const_cast<Font**>(&p->m_rgpFont[0])),
            m_pEnd(const_cast<Font**>(&p->m_rgpFont[p->m_cFonts])) {}

        public: bool AtEnd() const { return m_pRunner >= m_pEnd; }
        public: Font* Get() const { ASSERT(! AtEnd()); return *m_pRunner; }
        public: void Next() { ASSERT(! AtEnd()); m_pRunner++; }
    }; // EnumFont
}; // FontSet

#endif //!defined(INCLUDE_listener_winapp_visual_fontset_h)
