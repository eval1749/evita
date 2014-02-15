#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - FontSet class
// listener/winapp/fontset.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Style.cpp#2 $
//
#include "evita/vi_style.h"

#include <vector>

#include "evita/gfx_base.h"

namespace {
bool IsCacheableChar(char16 wch) {
  return wch >= 0x20 && wch <= 0x7E;
}

bool IsCachableString(const char16* pwch, size_t cwch) {
  for (auto s = pwch; s < pwch + cwch; ++s) {
    if (!IsCacheableChar(*s))
      return false;
  }
  return true;
}

}

// FontSet::Add
void FontSet::Add(Font* pFont)
{
    ASSERT(NULL != pFont);
    m_rgpFont[m_cFonts] = pFont;
    m_cFonts++;
} // FontSet::Add

//////////////////////////////////////////////////////////////////////
//
// Font
//
class Font::FontImpl {
  private: const std::unique_ptr<gfx::FontFace> font_face_;
  private: float const em_size_; // the logical size of the font in DIP units.
  private: float const pixels_per_dip_;
  private: const DWRITE_FONT_METRICS metrics_;

  public: FontImpl(const char16* face_name, int font_size_pt)
      : font_face_(new gfx::FontFace(face_name)),
        em_size_(font_size_pt * 96.0f / 72.0f),
        pixels_per_dip_(gfx::FactorySet::instance()->pixels_per_dip().height),
        metrics_(GetMetrics()) {
  }

  // [C]
  public: SimpleMetrics CalculateMetrics() const {
    SimpleMetrics metrics;
    metrics.ascent = ConvertToDip(metrics_.ascent);
    metrics.descent = ConvertToDip(metrics_.descent);
    metrics.height = ConvertToDip(metrics_.ascent +
                                  metrics_.descent +
                                  metrics_.lineGap);
    metrics.fixed_width = ConvertToDip(CalculateFixedWidth());
    return metrics;
  }

  private: uint CalculateFixedWidth() const {
    static char16 cacheable_chars[0x7E - 0x20 + 1];
    if (!cacheable_chars[0]) {
      for (int ch = ' '; ch <= 0x7E; ++ch) {
        cacheable_chars[ch - 0x20] = static_cast<char16>(ch);
      }
    }

    const auto metrics = GetGlyphMetrics(cacheable_chars,
                                         lengthof(cacheable_chars));
    auto const width = metrics[0].advanceWidth;
    for (const auto metric: metrics) {
      if (width != metric.advanceWidth)
        return 0;
    }
    return width;
  }

  public: float ConvertToDip(uint design_unit) const {
    return design_unit * em_size_ / metrics_.designUnitsPerEm;
  }

  public: float ConvertToDip(int design_unit) const {
    ASSERT(design_unit >= 0);
    return ConvertToDip(static_cast<uint>(design_unit));
  }

  // [D]
  public: void DrawText(const gfx::Graphics& gfx,const gfx::Brush& text_brush,
                        const gfx::PointF& baseline, const char16* chars,
                        uint num_chars) const {
    ASSERT(num_chars);
    const auto glyph_indexes = GetGlyphIndexes(chars, num_chars);

    std::vector<float> glyph_advances(num_chars);
    {
      const auto glyph_metrics = GetGlyphMetrics(glyph_indexes);
      auto metrics_it = glyph_metrics.begin();
      for (auto& it: glyph_advances) {
        it = ConvertToDip(metrics_it->advanceWidth);
        ++metrics_it;
      }
    }

    DWRITE_GLYPH_RUN glyph_run;
    glyph_run.fontFace = *font_face_;
    glyph_run.fontEmSize = em_size_;
    glyph_run.glyphCount = glyph_indexes.size();
    glyph_run.glyphIndices = &glyph_indexes[0];
    glyph_run.glyphAdvances = &glyph_advances[0];
    glyph_run.glyphOffsets = nullptr;
    glyph_run.isSideways = false;
    glyph_run.bidiLevel = 0;

    ASSERT(gfx.drawing());
    gfx->DrawGlyphRun(baseline, &glyph_run, text_brush,
                      DWRITE_MEASURING_MODE_GDI_NATURAL);
  }

  // [G]
  public: std::vector<uint16> GetGlyphIndexes(const char16* pwch,
                                             uint cwch) const {
    ASSERT(cwch);
    std::vector<uint32> code_points(cwch);
    auto it = code_points.begin();
    for (auto s = pwch; s < pwch + cwch; ++s) {
      *it = *s;
      ++it;
    }
    std::vector<uint16> glyph_indexes(cwch);
    COM_VERIFY((*font_face_)->GetGlyphIndices(
        &code_points[0], code_points.size(), &glyph_indexes[0]));
    return std::move(glyph_indexes);
  }

  public: std::vector<DWRITE_GLYPH_METRICS> GetGlyphMetrics(
      const char16* pwch, uint cwch) const {
    return GetGlyphMetrics(GetGlyphIndexes(pwch, cwch));
  }

  private: std::vector<DWRITE_GLYPH_METRICS> GetGlyphMetrics(
      const std::vector<uint16> glyph_indexes) const {
    DWRITE_MATRIX* const transform = nullptr;
    auto const use_gdi_natural = true;
    auto const is_side_ways = false;
    std::vector<DWRITE_GLYPH_METRICS> metrics(glyph_indexes.size());
    COM_VERIFY((*font_face_)->GetGdiCompatibleGlyphMetrics(
        em_size_, pixels_per_dip_, transform, use_gdi_natural,
        &glyph_indexes[0], glyph_indexes.size(), &metrics[0], is_side_ways));
    return std::move(metrics);
  }

  // [G]
  private: DWRITE_FONT_METRICS GetMetrics() {
    DWRITE_FONT_METRICS metrics;
    COM_VERIFY((*font_face_)->GetGdiCompatibleMetrics(
        em_size_, pixels_per_dip_, nullptr, &metrics));
    return metrics;
  }

  // [H]
  public: bool HasCharacter(char16 wch) const {
    uint32 code_point = wch;
    uint16 glyph_index;
    COM_VERIFY((*font_face_)->GetGlyphIndices(&code_point, 1, &glyph_index));
    return glyph_index;
  }

  DISALLOW_COPY_AND_ASSIGN(FontImpl);
};

Font::Font(const LOGFONT* log_font)
    : m_oLogFont(*log_font),
      font_impl_(new FontImpl(log_font->lfFaceName, log_font->lfHeight)),
      metrics_(font_impl_->CalculateMetrics()) {
}

Font::~Font() {
}

void Font::DrawText(const gfx::Graphics& gfx,const gfx::Brush& text_brush,
                    const gfx::RectF& rect, const char16* chars,
                    uint num_chars) const {
  auto const baseline = rect.left_top() + gfx::SizeF(0.0f, metrics_.ascent);
  font_impl_->DrawText(gfx, text_brush, baseline, chars, num_chars);
}

float Font::GetCharWidth(char16 wch) const {
  if (IsCacheableChar(wch) && metrics_.fixed_width)
    return metrics_.fixed_width;
  return GetTextWidth(&wch, 1);
}

float Font::GetTextWidth(const char16* pwch, size_t cwch) const {
  if (metrics_.fixed_width && IsCachableString(pwch, cwch))
    return metrics_.fixed_width * cwch;

  const auto metrics = font_impl_->GetGlyphMetrics(pwch, cwch);
  auto width = 0;
  for (const auto metric: metrics) {
    width += metric.advanceWidth;
  }
  return font_impl_->ConvertToDip(width);
}

bool Font::HasCharacter(char16 wch) const {
  // Note: The first font in FontSet must satisfiy this invariant.
  // TODO(yosi): We don't belive this assumption.l
  if (wch >= 0x20 && wch <= 0x7E)
    return true;
  return font_impl_->HasCharacter(wch);
}

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
Font* FontSet::FindFont(char16 wch) const {
  foreach (EnumFont, oEnum, this) {
    auto const pFont = oEnum.Get();
    if (pFont->HasCharacter(wch))
      return pFont;
  }
  return nullptr;
} // FontSet::FindFont

int memhash(const void* pv, size_t cb)
{
    ASSERT(NULL != pv);
    const uint* s = reinterpret_cast<const uint*>(pv);
    const uint* e = s + cb / sizeof(uint);
    uint nHashCode = 0;
    for (const uint* p = s; p < e; p++)
    {
        uint nHigh = nHashCode >>= sizeof(uint) * 8 - 5;
        nHashCode |= *p;
        nHashCode <<= 5;
        nHashCode |= nHigh;
    } // for
    return static_cast<int>(nHashCode & ((1<<28)-1)) & MAXINT;
} // memhash

int Font::HashKey(const Key* pKey)
    { return memhash(pKey, sizeof(*pKey)); }

int FontSet::HashKey(const Key* pKey)
{
    const Fonts* p = reinterpret_cast<const Fonts*>(pKey);
    ASSERT(p->m_cFonts >= 1);
    return memhash(p->m_rgpFont, sizeof(Font*) * p->m_cFonts);
} // FontSet::HashKey

template<class Item_, class Key_, int t_N = 31>
class Cache_
{
    private: struct Slot
    {
        Item_*  m_pItem;

        bool HasItem() const
        {
            return NULL != m_pItem && Removed() != m_pItem;
        } // HasItem
    }; // Slot

    private: Slot*  m_prgSlot;
    private: size_t m_cAlloc;
    private: size_t m_cItems;

    public: Cache_() :
        m_cAlloc(t_N),
        m_cItems(0),
        m_prgSlot(new Slot[t_N])
    {
        ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);
    } // Cache_

    private: static Item_* Removed()
        { return reinterpret_cast<Item_*>(1); };

    public: Item_* Get(const Key_* pKey) const
    {
        int iHashCode = Item_::HashKey(pKey);
        const Slot* pTop    = &m_prgSlot[0];
        const Slot* pBottom = &m_prgSlot[m_cAlloc];
        const Slot* pStart  = &m_prgSlot[iHashCode % m_cAlloc];
        const Slot* pRunner = pStart;
        do
        {
            Item_* pItem = pRunner->m_pItem;
            if (NULL == pItem)
            {
                return NULL;
            }

            if (Removed() == pItem)
            {
                // removed
            }
            else if (pItem->EqualKey(pKey))
            {
                return pItem;
            }

            pRunner++;
            if (pRunner == pBottom) pRunner = pTop;
        } while (pRunner != pStart);
        CAN_NOT_HAPPEN();
    } // Get

    public: void Put(Item_* pItem)
    {
        ASSERT(NULL != pItem);

        if (m_cItems * 60 > m_cAlloc * 100)
        {
            rehash();
        }

        int iHashCode = Item_::HashKey(pItem->GetKey());
        Slot* pTop    = &m_prgSlot[0];
        Slot* pBottom = &m_prgSlot[m_cAlloc];
        Slot* pStart  = &m_prgSlot[iHashCode % m_cAlloc];
        Slot* pRunner = pStart;
        Slot* pHome = NULL;
        const Key_* pKey = pItem->GetKey();
        do
        {
            Item_* pPresent = pRunner->m_pItem;
            if (NULL == pPresent)
            {
                if (NULL == pHome) pHome = pRunner;
                pHome->m_pItem = pItem;
                m_cItems += 1;
                return;
            }

            if (Removed() == pPresent)
            {
                if (NULL == pHome) pHome = pRunner;
            }
            else if (pPresent->EqualKey(pKey))
            {
                return;
            }

            pRunner++;
            if (pRunner == pBottom) pRunner = pTop;
        } while (pRunner != pStart);
        CAN_NOT_HAPPEN();
    } // Put

    private: void rehash()
    {
        Slot* prgStart = m_prgSlot;
        auto const cAllocs = m_cAlloc;
        auto cItems  = m_cItems;

        m_cAlloc = m_cAlloc * 130 / 100;
        m_cItems  = 0;
        m_prgSlot = new Slot[m_cAlloc];
        ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);

        Slot* prgEnd = prgStart + cAllocs;
        for (Slot* pRunner = prgStart; pRunner < prgEnd; pRunner++)
        {
            if (pRunner->HasItem())
            {
                Put(pRunner->m_pItem);
                cItems -= 1;
                if (0 == cItems) break;
            }
        } // for pRunner
    } // rehash
}; // Cache_

typedef Cache_<Font, Font::Key> FontCache;
typedef Cache_<FontSet, FontSet::Key> FontSetCache;
FontCache* g_pFontCache;
FontSetCache* g_pFontSetCache;


//////////////////////////////////////////////////////////////////////
//
// FontSet::Get
//
FontSet* FontSet::Get(const StyleValues* pStyle)
{
    Fonts oFonts;
    oFonts.m_cFonts = 0;

    const char16* pwszFamily = pStyle->GetFontFamily();
    while (0 != *pwszFamily)
    {
        LOGFONT oLogFont;
        ::ZeroMemory(&oLogFont, sizeof(oLogFont));

        oLogFont.lfHeight = pStyle->m_nFontSize;
        oLogFont.lfWidth = 0;
        oLogFont.lfEscapement = 0;
        oLogFont.lfOrientation = 0;

        oLogFont.lfWeight =
            FontWeight_Bold == pStyle->GetFontWeight() ? FW_BOLD : FW_NORMAL;

        oLogFont.lfItalic =
            FontStyle_Italic == pStyle->GetFontStyle() ? 1u : 0u;

        oLogFont.lfUnderline =
            TextDecoration_Underline == pStyle->GetDecoration() ? 1u : 0u;

        oLogFont.lfStrikeOut     = 0;
        oLogFont.lfCharSet       = ANSI_CHARSET;;
        oLogFont.lfOutPrecision  = OUT_DEFAULT_PRECIS;
        oLogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        oLogFont.lfQuality       = DEFAULT_QUALITY;

        // We use FIXED_PITCH. This makes width of Kanji character is double
        // of width of alphabet character.
        //oLogFont.lfPitchAndFamily = DEFAULT_PITCH;
        oLogFont.lfPitchAndFamily = FIXED_PITCH;

        while (IsWhitespace(*pwszFamily))
        {
            pwszFamily++;
        } // while

        char16* pwsz = oLogFont.lfFaceName;
        while (0 != *pwszFamily) {
            if (',' == *pwszFamily)
            {
                pwszFamily++;
                break;
            }

            *pwsz++ = *pwszFamily++;
        } // while

        if (NULL == g_pFontCache)
        {
            g_pFontCache = new FontCache;
        }

        Font* pFont = g_pFontCache->Get(&oLogFont);
        if (!pFont) {
            pFont = new Font(&oLogFont);
            g_pFontCache->Put(pFont);
        }

        oFonts.m_rgpFont[oFonts.m_cFonts] = pFont;

        oFonts.m_cFonts += 1;
    } // for

    if (NULL == g_pFontSetCache)
    {
        g_pFontSetCache = new FontSetCache;
    }

    FontSet* pFontSet = g_pFontSetCache->Get(&oFonts);

    if (NULL == pFontSet)
    {
        pFontSet = new FontSet;
        for (int i = 0; i < oFonts.m_cFonts; i++)
        {
            pFontSet->Add(oFonts.m_rgpFont[i]);
        } // for i
        g_pFontSetCache->Put(pFontSet);
    }
    return pFontSet;
} // FontSet::Get
