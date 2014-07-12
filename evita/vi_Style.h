// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_style_h)
#define INCLUDE_evita_vi_style_h

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
    float underline;
    float underline_thickness;
  };

  private: LOGFONT log_font_;
  private: const std::unique_ptr<FontImpl> font_impl_;
  private: const SimpleMetrics metrics_;

  public: Font(const LOGFONT* logFont);
  public: ~Font();

  private: float ascent() const { return metrics_.ascent; }
  public: float descent() const { return metrics_.descent; }
  public: float height() const { return metrics_.height; }
  public: float underline() const { return metrics_.underline; }
  public: float underline_thickness() const {
    return metrics_.underline_thickness;
  }

  // [D]
  public: void DrawText(gfx::Canvas* canvas, const gfx::Brush& text_brush,
                        const gfx::RectF& rect, const base::char16* chars,
                        uint num_chars) const;
  public: void DrawText(gfx::Canvas* canvas,const gfx::Brush& text_brush,
                        const gfx::RectF& rect,
                        const base::string16& string) const;

  // [E]
  public: bool EqualKey(const Key* pKey) const {
    return !::memcmp(&log_font_, pKey, sizeof(log_font_));
  }

  // [G]
  public: float GetCharWidth(char16) const;
  public: const Key* GetKey() const { return &log_font_; }
  public: float GetTextWidth(const base::char16* pwch, size_t cwch) const;
  public: float GetTextWidth(const base::string16& string) const;

  // [H]
  public: bool HasCharacter(char16) const;

  public: uint Hash() const {
    return static_cast<uint>(reinterpret_cast<UINT_PTR>(this));
  }

  public: static int HashKey(const Key*);

  DISALLOW_COPY_AND_ASSIGN(Font);
};

//////////////////////////////////////////////////////////////////////
//
// FontSet class
//
struct Fonts {
  Font* fonts_[10];
  int num_fonts_;
};

class FontSet : public Fonts {
  public: typedef Fonts Key;

  public: FontSet() {
      num_fonts_ = 0;
  }

  // [A]
  public: void Add(Font*);

  // [E]
  public: bool EqualKey(const Key* pFonts) const;

  // [F]
  public: Font* FindFont(char16) const;

  // [G]
  public: static FontSet* Get(const css::Style& style);
  public: const Key* GetKey() const { return this; }

  // [H]
  public: static int HashKey(const Key*);

  // [E]
  public: class EnumFont {
    Font**  m_pRunner;
    Font**  m_pEnd;

    public: EnumFont(const FontSet* p) :
        m_pRunner(const_cast<Font**>(&p->fonts_[0])),
        m_pEnd(const_cast<Font**>(&p->fonts_[p->num_fonts_])) {}

    public: bool AtEnd() const { return m_pRunner >= m_pEnd; }
    public: Font* Get() const { ASSERT(! AtEnd()); return *m_pRunner; }
    public: void Next() { ASSERT(! AtEnd()); m_pRunner++; }
  };
};

#endif //!defined(INCLUDE_evita_vi_style_h)
