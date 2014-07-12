#include "precomp.h"
// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_style.h"

#include <vector>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/font_face.h"
#include "evita/css/style.h"

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
void FontSet::Add(Font* font)
{
    ASSERT(NULL != font);
    fonts_[num_fonts_] = font;
    num_fonts_++;
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
    metrics.underline = ConvertToDip(-metrics_.underlinePosition);
    metrics.underline_thickness = ConvertToDip(metrics_.underlineThickness);
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
                                         arraysize(cacheable_chars));
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
  public: void DrawText(gfx::Canvas* canvas, const gfx::Brush& text_brush,
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

    ASSERT(canvas->drawing());
    (*canvas)->DrawGlyphRun(baseline, &glyph_run, text_brush,
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
    : log_font_(*log_font),
      font_impl_(new FontImpl(log_font->lfFaceName, log_font->lfHeight)),
      metrics_(font_impl_->CalculateMetrics()) {
}

Font::~Font() {
}

void Font::DrawText(gfx::Canvas* canvas,const gfx::Brush& text_brush,
                    const gfx::RectF& rect, const base::char16* chars,
                    uint num_chars) const {
  auto const baseline = rect.left_top() + gfx::SizeF(0.0f, metrics_.ascent);
  font_impl_->DrawText(canvas, text_brush, baseline, chars, num_chars);
}

void Font::DrawText(gfx::Canvas* canvas,const gfx::Brush& text_brush,
                    const gfx::RectF& rect,
                    const base::string16& string) const {
  DrawText(canvas, text_brush, rect, string.data(),
           static_cast<uint32>(string.length()));
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

float Font::GetTextWidth(const base::string16& string) const {
  return GetTextWidth(string.data(), static_cast<uint32>(string.length()));
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
    auto const font = oEnum.Get();
    if (font->HasCharacter(wch))
      return font;
  }
  return nullptr;
} // FontSet::FindFont

int memhash(const void* pv, size_t cb) {
  DCHECK(pv);
  const uint* s = reinterpret_cast<const uint*>(pv);
  const uint* e = s + cb / sizeof(uint);
  uint nHashCode = 0;
  for (const uint* p = s; p < e; p++) {
    uint nHigh = nHashCode >>= sizeof(uint) * 8 - 5;
    nHashCode |= *p;
    nHashCode <<= 5;
    nHashCode |= nHigh;
  }
  return static_cast<int>(nHashCode & ((1<<28)-1)) & MAXINT;
}

int Font::HashKey(const Key* pKey) {
  return memhash(pKey, sizeof(*pKey));
}

bool FontSet::EqualKey(const Key* fonts) const {
  if (fonts->num_fonts_ != num_fonts_) return false;
  DCHECK_LT(num_fonts_, static_cast<int>(arraysize(fonts_)));
  return !::memcmp(fonts->fonts_, fonts_,
                   sizeof(fonts_[0]) * num_fonts_);
}

int FontSet::HashKey(const Key* pKey) {
  const Fonts* p = reinterpret_cast<const Fonts*>(pKey);
  DCHECK_GE(p->num_fonts_, 1);
  return memhash(p->fonts_, sizeof(Font*) * p->num_fonts_);
}

template<class Item_, class Key_, int t_N = 31>
class Cache_ {
  private: struct Slot {
    Item_*  m_pItem;

    bool HasItem() const {
      return m_pItem && Removed() != m_pItem;
    }
  };

  private: Slot*  m_prgSlot;
  private: size_t m_cAlloc;
  private: size_t m_cItems;

  public: Cache_()
    : m_cAlloc(t_N), m_cItems(0), m_prgSlot(new Slot[t_N]) {
    ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);
  }

  private: static Item_* Removed() {
    return reinterpret_cast<Item_*>(1);
  }

  public: Item_* Get(const Key_* pKey) const {
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

  public: void Put(Item_* pItem) {
    DCHECK(pItem);

    if (m_cItems * 60 > m_cAlloc * 100)
      rehash();

    int iHashCode = Item_::HashKey(pItem->GetKey());
    Slot* pTop    = &m_prgSlot[0];
    Slot* pBottom = &m_prgSlot[m_cAlloc];
    Slot* pStart  = &m_prgSlot[iHashCode % m_cAlloc];
    Slot* pRunner = pStart;
    Slot* pHome = NULL;
    const Key_* pKey = pItem->GetKey();
    do {
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
  }

  private: void rehash() {
    Slot* prgStart = m_prgSlot;
    auto const cAllocs = m_cAlloc;
    auto cItems  = m_cItems;

    m_cAlloc = m_cAlloc * 130 / 100;
    m_cItems  = 0;
    m_prgSlot = new Slot[m_cAlloc];
    ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);

    Slot* prgEnd = prgStart + cAllocs;
    for (Slot* pRunner = prgStart; pRunner < prgEnd; pRunner++) {
      if (pRunner->HasItem()) {
        Put(pRunner->m_pItem);
        cItems -= 1;
        if (!cItems)
          break;
      }
    }
  }
};

typedef Cache_<Font, Font::Key> FontCache;
typedef Cache_<FontSet, FontSet::Key> FontSetCache;
FontCache* g_fontCache;
FontSetCache* g_font_setCache;

//////////////////////////////////////////////////////////////////////
//
// FontSet::Get
//
FontSet* FontSet::Get(const css::Style& style) {
  Fonts fonts;
  fonts.num_fonts_ = 0;

  auto pwszFamily = style.font_family().data();
  while (*pwszFamily) {
    LOGFONT log_font;
    ::ZeroMemory(&log_font, sizeof(log_font));

    log_font.lfHeight = static_cast<LONG>(style.font_size());
    log_font.lfWidth = 0;
    log_font.lfEscapement = 0;
    log_font.lfOrientation = 0;

    log_font.lfWeight =
        css::FontWeight::Bold == style.font_weight() ? FW_BOLD : FW_NORMAL;

    log_font.lfItalic =
        css::FontStyle::Italic == style.font_style() ? 1u : 0u;

    log_font.lfUnderline =
        css::TextDecoration::Underline == style.text_decoration() ? 1u : 0u;

    log_font.lfStrikeOut = 0;
    log_font.lfCharSet = ANSI_CHARSET;;
    log_font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    log_font.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    log_font.lfQuality = DEFAULT_QUALITY;

    // We use FIXED_PITCH. This makes width of Kanji character is double
    // of width of alphabet character.
    //log_font.lfPitchAndFamily = DEFAULT_PITCH;
    log_font.lfPitchAndFamily = FIXED_PITCH;

    while (IsWhitespace(*pwszFamily)) {
      pwszFamily++;
    }

    auto pwsz = log_font.lfFaceName;
    while (*pwszFamily) {
      if (',' == *pwszFamily) {
        pwszFamily++;
        break;
      }

      *pwsz++ = *pwszFamily++;
    }

    if (!g_fontCache)
      g_fontCache = new FontCache;

    auto font = g_fontCache->Get(&log_font);
    if (!font) {
      font = new Font(&log_font);
      g_fontCache->Put(font);
    }

    fonts.fonts_[fonts.num_fonts_] = font;

    fonts.num_fonts_ += 1;
  }

  if (!g_font_setCache)
      g_font_setCache = new FontSetCache;

  FontSet* font_set = g_font_setCache->Get(&fonts);
  if (!font_set) {
    font_set = new FontSet;
    for (auto i = 0; i < fonts.num_fonts_; i++) {
      font_set->Add(fonts.fonts_[i]);
    }
    g_font_setCache->Put(font_set);
  }
  return font_set;
}
