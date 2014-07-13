#include "precomp.h"
// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_style.h"

#include <unordered_map>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "common/memory/singleton.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/font_face.h"
#include "evita/css/style.h"

namespace {

gfx::FontProperties ComputeFontProperties(const base::string16& family_name,
                                          const css::Style& style) {
  gfx::FontProperties font_props;
  font_props.bold = css::FontWeight::Bold == style.font_weight();
  font_props.font_size_pt = style.font_size();
  font_props.italic = css::FontStyle::Italic == style.font_style();
  font_props.family_name = family_name;
  return font_props;
}

bool IsCacheableChar(char16 wch) {
  return wch >= 0x20 && wch <= 0x7E;
}

bool IsCachableString(const base::char16* pwch, size_t cwch) {
  for (auto s = pwch; s < pwch + cwch; ++s) {
    if (!IsCacheableChar(*s))
      return false;
  }
  return true;
}

std::vector<base::string16> parseFontFamily(const base::string16& source) {
  enum class State{
    Comma,
    Name,
    NameSpace,
    Start,
  } state = State::Start;
  std::vector<base::string16> names;
  base::string16 name;
  for (auto const ch : source) {
    switch (state) {
      case State::Name:
        if (ch == ',') {
          names.push_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (IsWhitespace(ch))
          state = State::NameSpace;
        else
          name.push_back(ch);
        break;
      case State::NameSpace:
        if (ch == ',') {
          names.push_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (!IsWhitespace(ch)) {
          name.push_back(' ');
          name.push_back(ch);
          state = State::Name;
        }
        break;
      case State::Start:
        if (ch != ',' && !IsWhitespace(ch)) {
          name.push_back(ch);
          state = State::Name;
        }
        break;
    }
  }
  if (name.length())
    names.push_back(name);
  return names;
}

}  // namespace

namespace std {
template<> struct hash<views::rendering::FontSet::FontList> {
  size_t operator()(const views::rendering::FontSet::FontList& fonts) const {
    size_t result = 137u;
    for (auto const font : fonts) {
      result <<= 1;
      result ^= hash<const views::rendering::Font*>()(font);
    }
    return result;
  }
};
}  // namespace std

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// Font::Cache
//
class Font::Cache : public common::Singleton<Font::Cache> {
  DECLARE_SINGLETON_CLASS(Font::Cache);

  private: std::unordered_map<gfx::FontProperties, Font*> map_;

  private: Cache() = default;
  private: ~Cache() = default;

  public: const Font& GetOrCreate(const gfx::FontProperties& font_props);
};

const Font& Font::Cache::GetOrCreate(const gfx::FontProperties& font_props) {
  auto const present = map_.find(font_props);
  if (present != map_.end())
    return *present->second;
  auto new_font = new Font(font_props);
  map_[font_props] = new_font;
  return *new_font;
}

//////////////////////////////////////////////////////////////////////
//
// FontSet::Cache
//
class FontSet::Cache : public common::Singleton<FontSet::Cache> {
  DECLARE_SINGLETON_CLASS(FontSet::Cache);

  private: std::unordered_map<FontList, FontSet*> map_;

  private: Cache() = default;
  private: ~Cache() = default;

  public: const FontSet& GetOrCreate(const FontList& fonts);
};

const FontSet& FontSet::Cache::GetOrCreate(const FontList& fonts) {
  auto const present = map_.find(fonts);
  if (present != map_.end())
    return *present->second;
  auto new_font_set = new FontSet(fonts);
  map_[fonts] = new_font_set;
  return *new_font_set;
}

//////////////////////////////////////////////////////////////////////
//
// Font
//
class Font::FontImpl {
  private: const std::unique_ptr<gfx::FontFace> font_face_;
  private: float const em_size_; // the logical size of the font in DIP units.
  private: float const pixels_per_dip_;
  private: const DWRITE_FONT_METRICS metrics_;

  public: FontImpl(const gfx::FontProperties& properties);

  // [C]
  public: SimpleMetrics CalculateMetrics() const;
  private: uint32_t CalculateFixedWidth() const;
  public: float ConvertToDip(uint32_t design_unit) const;
  public: float ConvertToDip(int design_unit) const;

  // [D]
  public: void DrawText(gfx::Canvas* canvas, const gfx::Brush& text_brush,
                        const gfx::PointF& baseline, const base::char16* chars,
                        size_t num_chars) const;

  // [G]
  public: std::vector<uint16_t> GetGlyphIndexes(const base::char16* chars,
                                                size_t num_chars) const;
  public: std::vector<DWRITE_GLYPH_METRICS> GetGlyphMetrics(
      const base::char16* chars, size_t num_chars) const;
  private: std::vector<DWRITE_GLYPH_METRICS> GetGlyphMetrics(
      const std::vector<uint16_t> glyph_indexes) const;
  private: DWRITE_FONT_METRICS GetMetrics() const;

  // [H]
  public: bool HasCharacter(base::char16 sample) const;

  DISALLOW_COPY_AND_ASSIGN(FontImpl);
};

Font::FontImpl::FontImpl(const gfx::FontProperties& properties)
    : font_face_(new gfx::FontFace(properties)),
      em_size_(properties.font_size_pt * 96.0f / 72.0f),
      pixels_per_dip_(gfx::FactorySet::instance()->pixels_per_dip().height),
      metrics_(GetMetrics()) {
}

Font::SimpleMetrics Font::FontImpl::CalculateMetrics() const {
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

uint32_t Font::FontImpl::CalculateFixedWidth() const {
  static base::char16 cacheable_chars[0x7E - 0x20 + 1];
  if (!cacheable_chars[0]) {
    for (int ch = ' '; ch <= 0x7E; ++ch) {
      cacheable_chars[ch - 0x20] = static_cast<char16>(ch);
    }
  }

  const auto metrics = GetGlyphMetrics(cacheable_chars,
                                       arraysize(cacheable_chars));
  auto const width = metrics[0].advanceWidth;
  for (const auto metric : metrics) {
    if (width != metric.advanceWidth)
      return 0u;
  }
  return width;
}

float Font::FontImpl::ConvertToDip(uint32_t design_unit) const {
  return design_unit * em_size_ / metrics_.designUnitsPerEm;
}

float Font::FontImpl::ConvertToDip(int design_unit) const {
  DCHECK_GE(design_unit, 0);
  return ConvertToDip(static_cast<uint32_t>(design_unit));
}

void Font::FontImpl::DrawText(
    gfx::Canvas* canvas, const gfx::Brush& text_brush,
    const gfx::PointF& baseline, const base::char16* chars,
    size_t num_chars) const {
  DCHECK_GE(num_chars, 1u);
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

  DCHECK(canvas->drawing());
  (*canvas)->DrawGlyphRun(baseline, &glyph_run, text_brush,
                          DWRITE_MEASURING_MODE_GDI_NATURAL);
}

std::vector<uint16_t> Font::FontImpl::GetGlyphIndexes(
    const base::char16* chars, size_t num_chars) const {
  DCHECK_GE(num_chars, 1u);
  std::vector<uint32> code_points(num_chars);
  auto it = code_points.begin();
  for (auto s = chars; s < chars + num_chars; ++s) {
    *it = *s;
    ++it;
  }
  std::vector<uint16> glyph_indexes(num_chars);
  COM_VERIFY((*font_face_)->GetGlyphIndices(
      &code_points[0], code_points.size(), &glyph_indexes[0]));
  return std::move(glyph_indexes);
}

std::vector<DWRITE_GLYPH_METRICS> Font::FontImpl::GetGlyphMetrics(
    const base::char16* chars, size_t num_chars) const {
  return GetGlyphMetrics(GetGlyphIndexes(chars, num_chars));
}

std::vector<DWRITE_GLYPH_METRICS> Font::FontImpl::GetGlyphMetrics(
    const std::vector<uint16_t> glyph_indexes) const {
  DWRITE_MATRIX* const transform = nullptr;
  auto const use_gdi_natural = true;
  auto const is_side_ways = false;
  std::vector<DWRITE_GLYPH_METRICS> metrics(glyph_indexes.size());
  COM_VERIFY((*font_face_)->GetGdiCompatibleGlyphMetrics(
      em_size_, pixels_per_dip_, transform, use_gdi_natural,
      &glyph_indexes[0], glyph_indexes.size(), &metrics[0], is_side_ways));
  return std::move(metrics);
}

DWRITE_FONT_METRICS Font::FontImpl::GetMetrics() const {
  DWRITE_FONT_METRICS metrics;
  COM_VERIFY((*font_face_)->GetGdiCompatibleMetrics(
      em_size_, pixels_per_dip_, nullptr, &metrics));
  return metrics;
}

bool Font::FontImpl::HasCharacter(base::char16 sample) const {
  uint32_t code_point = sample;
  uint16_t glyph_index;
  COM_VERIFY((*font_face_)->GetGlyphIndices(&code_point, 1, &glyph_index));
  return glyph_index;
}

//////////////////////////////////////////////////////////////////////
//
// Font
//
Font::Font(const gfx::FontProperties& properties)
    : font_impl_(new FontImpl(properties)),
      metrics_(font_impl_->CalculateMetrics()) {
}

Font::~Font() {
}

void Font::DrawText(gfx::Canvas* canvas, const gfx::Brush& text_brush,
                    const gfx::RectF& rect, const base::char16* chars,
                    uint32_t num_chars) const {
  auto const baseline = rect.left_top() + gfx::SizeF(0.0f, metrics_.ascent);
  font_impl_->DrawText(canvas, text_brush, baseline, chars, num_chars);
}

void Font::DrawText(gfx::Canvas* canvas,const gfx::Brush& text_brush,
                    const gfx::RectF& rect,
                    const base::string16& string) const {
  DrawText(canvas, text_brush, rect, string.data(),
           static_cast<uint32>(string.length()));
}

const Font& Font::Get(const gfx::FontProperties& properties) {
  return Cache::instance()->GetOrCreate(properties);
}

float Font::GetCharWidth(char16 wch) const {
  if (IsCacheableChar(wch) && metrics_.fixed_width)
    return metrics_.fixed_width;
  return GetTextWidth(&wch, 1);
}

float Font::GetTextWidth(const base::char16* chars, size_t num_chars) const {
  if (metrics_.fixed_width && IsCachableString(chars, num_chars))
    return metrics_.fixed_width * num_chars;

  const auto metrics = font_impl_->GetGlyphMetrics(chars, num_chars);
  auto width = 0;
  for (const auto metric: metrics) {
    width += metric.advanceWidth;
  }
  return font_impl_->ConvertToDip(width);
}

float Font::GetTextWidth(const base::string16& string) const {
  return GetTextWidth(string.data(), static_cast<uint32>(string.length()));
}

bool Font::HasCharacter(char16 sample) const {
  // Note: The first font in FontSet must satisfy this invariant.
  // TODO(yosi): We don't believe this assumption.l
  if (sample >= 0x20 && sample <= 0x7E)
    return true;
  return font_impl_->HasCharacter(sample);
}

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
FontSet::FontSet(const std::vector<const Font*>& fonts) : fonts_(fonts) {
}

FontSet::~FontSet() {
}

const Font* FontSet::FindFont(base::char16 sample) const {
  for (auto const font : fonts_) {
    if (font->HasCharacter(sample))
      return font;
  }
  return nullptr;
}

const FontSet& FontSet::Get(const css::Style& style) {
  FontList fonts;
  for (auto const font_family : parseFontFamily(style.font_family())) {
    const auto font_props = ComputeFontProperties(font_family, style);
    fonts.push_back(&Font::Get(font_props));
  }

  return FontSet::Cache::instance()->GetOrCreate(fonts);
}

const Font* FontSet::GetFont(const css::Style& style, base::char16 sample) {
  return Get(style).FindFont(sample);
}

}  // namespace rendering
}  // namespace views

namespace std {
size_t hash<views::rendering::Font>::operator()(
    const views::rendering::Font& font) const {
  return hash<const views::rendering::Font*>()(&font);
}
}  // namespace std
