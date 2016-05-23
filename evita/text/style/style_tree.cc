// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/style_tree.h"

#include "base/strings/string_util.h"
#include "base/trace_event/trace_event.h"
#include "evita/css/selector.h"
#include "evita/css/style.h"
#include "evita/css/values.h"
#include "evita/gfx/base/colors/float_color.h"
#include "evita/gfx/font.h"
#include "evita/gfx/font_face.h"
#include "evita/text/style/computed_style.h"
#include "evita/text/style/computed_style_builder.h"
#include "evita/visuals/style/compiled_style_sheet_set.h"

namespace layout {

namespace {

// TODO(eval1749): We should have css::StringListValue and make "font-family"
// to use it.
std::vector<base::string16> ParseFontFamily(base::StringPiece16 source) {
  enum class State {
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
          names.emplace_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (base::IsAsciiWhitespace(ch))
          state = State::NameSpace;
        else
          name.push_back(ch);
        break;
      case State::NameSpace:
        if (ch == ',') {
          names.emplace_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (!base::IsAsciiWhitespace(ch)) {
          name.push_back(' ');
          name.push_back(ch);
          state = State::Name;
        }
        break;
      case State::Start:
        if (ch != ',' && !base::IsAsciiWhitespace(ch)) {
          name.push_back(ch);
          state = State::Name;
        }
        break;
    }
  }
  if (name.length() > 0)
    names.emplace_back(name);
  return std::move(names);
}

gfx::ColorF ComputeColor(const css::ColorValue& color_value) {
  const auto& color = color_value.value();
  return gfx::ColorF(color.red(), color.green(), color.blue(), color.alpha());
}

TextDecorationLine ComputeTextDecorationLine(
    const css::TextDecorationLine& text_decoration) {
  if (text_decoration.is_none())
    return TextDecorationLine::None;
  if (text_decoration.is_line_through())
    return TextDecorationLine::LineThrough;
  if (text_decoration.is_overline())
    return TextDecorationLine::Overline;
  if (text_decoration.is_underline())
    return TextDecorationLine::Underline;
  NOTREACHED() << "Bad text-decoration-line: " << text_decoration;
  return TextDecorationLine::None;
}

TextDecorationStyle ComputeTextDecorationStyle(
    const css::TextDecorationStyle& text_decoration) {
  if (text_decoration.is_dashed())
    return TextDecorationStyle::Dashed;
  if (text_decoration.is_dotted())
    return TextDecorationStyle::Dotted;
  if (text_decoration.is_double())
    return TextDecorationStyle::Double;
  if (text_decoration.is_solid())
    return TextDecorationStyle::Solid;
  if (text_decoration.is_wavy())
    return TextDecorationStyle::Wavy;
  NOTREACHED() << "Bad text-decoration-style: " << text_decoration;
  return TextDecorationStyle::Solid;
}

std::unique_ptr<ComputedStyle> ComputeStyle(const css::Style& css_style,
                                            float zoom) {
  ComputedStyle::Builder builder;
  if (css_style.has_color())
    builder.SetColor(ComputeColor(css_style.color().as_color_value()));
  if (css_style.has_background_color()) {
    builder.SetBackgroundColor(
        ComputeColor(css_style.background_color().as_color_value()));
  }

  if (css_style.has_text_decoration_color()) {
    builder.SetTextDecorationColor(
        ComputeColor(css_style.text_decoration_color().as_color_value()));
  } else {
    // Default value is "color"
    builder.SetTextDecorationColor(builder.color());
  }

  if (css_style.has_text_decoration_line()) {
    builder.SetTextDecorationLine(
        ComputeTextDecorationLine(css_style.text_decoration_line()));
  }

  if (css_style.has_text_decoration_style()) {
    builder.SetTextDecorationStyle(
        ComputeTextDecorationStyle(css_style.text_decoration_style()));
  }

  if (css_style.has_font_family()) {
    gfx::FontProperties props;
    if (css_style.has_font_weight())
      props.bold = !css_style.font_weight().is_normal();
    if (css_style.has_font_style())
      props.italic = !css_style.font_style().is_normal();
    props.font_size_pt = css_style.font_size().as_length().value() * zoom;
    DCHECK_GT(props.font_size_pt, 0.0f);
    const auto& names =
        ParseFontFamily(css_style.font_family().as_string().data());
    std::vector<const gfx::Font*> fonts(names.size());
    fonts.resize(0);
    for (const auto& name : names) {
      props.family_name = name;
      fonts.emplace_back(&gfx::Font::Get(props));
    }
    builder.SetFonts(fonts);
  }

  return std::move(builder.Build());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Style Tree
//
StyleTree::StyleTree(const std::vector<css::StyleSheet*> style_sheets)
    : style_sheet_set_(new CompiledStyleSheetSet(style_sheets)) {}

StyleTree::~StyleTree() = default;

void StyleTree::ResetCache() {
  style_cache_.clear();
}

const ComputedStyle& StyleTree::ComputedStyleOf(
    const css::Selector& selector) const {
  const auto& it = style_cache_.find(selector);
  if (it != style_cache_.end())
    return *it->second;
  TRACE_EVENT0("view", "StyleTree::ComputedStyleOf");
  const auto& css_style = std::make_unique<css::Style>();
  style_sheet_set_->Merge(css_style.get(), selector);
  DCHECK(css_style->has_background_color()) << "No background-color for "
                                            << selector << ' ' << *css_style;
  DCHECK(css_style->has_color()) << "No color for " << selector << ' '
                                 << *css_style;
  DCHECK(css_style->has_font_family()) << "No font-family for " << selector
                                       << ' ' << *css_style;
  DCHECK(css_style->has_font_size()) << "No font-size for " << selector << ' '
                                     << *css_style;
  auto style = ComputeStyle(*css_style, zoom_);
  DCHECK(!style->fonts().empty()) << "No fonts for " << selector << ' '
                                  << *css_style;
  const auto& result = style_cache_.emplace(selector, std::move(style));
  DCHECK(result.second) << "must not be in cache " << selector;
  return *result.first->second;
}

void StyleTree::SetZoom(float new_zoom) {
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
  ResetCache();
}

// css::StyleSheetObserver
void StyleTree::DidInsertRule(const css::Rule& new_rule, size_t index) {
  ResetCache();
}

void StyleTree::DidRemoveRule(const css::Rule& old_rule, size_t index) {
  ResetCache();
}

}  // namespace layout
