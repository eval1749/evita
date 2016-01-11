// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "evita/dom/visuals/css_value_parsers.h"

#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "evita/visuals/css/background.h"
#include "evita/visuals/css/border.h"
#include "evita/visuals/css/color.h"
#include "evita/visuals/css/margin.h"
#include "evita/visuals/css/values.h"

namespace dom {

template <typename T>
using Maybe = common::Maybe<T>;

#define V(Name, name, text) using Css##Name = visuals::css::Name;
FOR_EACH_VISUAL_CSS_VALUES(V)
#undef V

using CssLength = visuals::css::Length;
using CssPercentage = visuals::css::Percentage;
using CssString = visuals::css::String;

namespace {

Maybe<int> ParseHex(const base::StringPiece16& text) {
  auto value = 0;
  for (const auto& code : text) {
    if (!base::IsHexDigit(code))
      return common::Nothing<int>();
    value = value * 16 + base::HexDigitToInt(code);
  }
  return common::Just<int>(value);
}

}  // namespace

Maybe<CssBackground> ParseBackground(const base::StringPiece16& text) {
  return common::Nothing<CssBackground>();
}

Maybe<CssBorder> ParseBorder(const base::StringPiece16& text) {
  return common::Nothing<CssBorder>();
}

Maybe<CssColor> ParseColor(const base::StringPiece16& text) {
  if (text.size() == 0)
    return common::Nothing<CssColor>();
  if (text[0] == '#') {
    if (text.size() == 4) {
      // #rgb
      const auto& maybe_red = ParseHex(text.substr(1, 1));
      const auto& maybe_green = ParseHex(text.substr(2, 1));
      const auto& maybe_blue = ParseHex(text.substr(3, 1));
      if (maybe_red.IsJust() && maybe_green.IsJust() && maybe_blue.IsJust())
        return common::Just<CssColor>(
            CssColor(static_cast<float>(maybe_red.FromJust() * 16) / 255,
                     static_cast<float>(maybe_green.FromJust() * 16) / 255,
                     static_cast<float>(maybe_blue.FromJust() * 16) / 255));
    }
    if (text.size() == 7) {
      // #rrggbb
      const auto& maybe_red = ParseHex(text.substr(1, 2));
      const auto& maybe_green = ParseHex(text.substr(3, 2));
      const auto& maybe_blue = ParseHex(text.substr(5, 2));
      if (maybe_red.IsJust() && maybe_green.IsJust() && maybe_blue.IsJust())
        return common::Just<CssColor>(
            CssColor(static_cast<float>(maybe_red.FromJust()) / 255,
                     static_cast<float>(maybe_green.FromJust()) / 255,
                     static_cast<float>(maybe_blue.FromJust()) / 255));
    }
  }
  if (text == L"transparent")
    return common::Just<CssColor>(CssColor());
  // TODO(eval1749): Parse color name
  // TODO(eval1749): Parse 'rgba(red, green, blue, alpha)
  return common::Nothing<CssColor>();
}

Maybe<CssLength> ParseLength(const base::StringPiece16& text) {
  return common::Just<CssLength>(CssLength(0));
}

Maybe<CssMargin> ParseMargin(const base::StringPiece16& text) {
  return common::Nothing<CssMargin>();
}

Maybe<CssPadding> ParsePadding(const base::StringPiece16& text) {
  return common::Nothing<CssPadding>();
}

Maybe<CssPercentage> ParsePercentage(const base::StringPiece16& text) {
  return common::Nothing<CssPercentage>();
}

Maybe<CssString> ParseString(const base::StringPiece16& text) {
  return common::Just<CssString>(CssString(text));
}

base::string16 UnparseBackground(const CssBackground& background) {
  if (!background.HasValue())
    return L"";
  return L"NYI(background)";
}

base::string16 UnparseBorder(const CssBorder& border) {
  if (!border.HasValue())
    return L"";
  std::basic_ostringstream<base::char16> ostream;
  if (border.IsSimple()) {
    ostream << L"border: " << border.top() << L" solid "
            << UnparseColor(border.top_color());
    return ostream.str();
  }
  base::char16* delimiter = L"";
  if (border.top() != 0.0f) {
    ostream << delimiter << L"border-top:" << border.top() << L" solid "
            << UnparseColor(border.top_color());
    delimiter = L"; ";
  }
  if (border.left() != 0.0f) {
    ostream << delimiter << L"border-left:" << border.left() << L" solid "
            << UnparseColor(border.left_color());
    delimiter = L"; ";
  }
  if (border.bottom() != 0.0f) {
    ostream << delimiter << L"border-bottom:" << border.bottom() << L" solid "
            << UnparseColor(border.bottom_color());
    delimiter = L"; ";
  }
  if (border.right() != 0.0f) {
    ostream << delimiter << L"border-right:" << border.right() << L" solid "
            << UnparseColor(border.right_color());
    delimiter = L"; ";
  }
  return ostream.str();
}

base::string16 UnparseColor(const CssColor& color) {
  const auto red = static_cast<int>(color.value().red() * 255);
  const auto green = static_cast<int>(color.value().green() * 255);
  const auto blue = static_cast<int>(color.value().blue() * 255);
  const auto alpha = color.value().alpha();
  if (alpha == 1.0f)
    return base::StringPrintf(L"#%02X%02X%02X", red, green, blue);
  if (alpha == 0 && red == 0 && green == 0 && blue == 0)
    return L"transparent";
  return base::StringPrintf(L"rgba(%d, %d, %d, %f)", red, green, blue, alpha);
}

base::string16 UnparseLength(const visuals::css::Length& value) {
  return base::StringPrintf(L"%f", value.value());
}

base::string16 UnparseMargin(const CssMargin& margin) {
  if (!margin.HasValue())
    return L"";
  return L"NYI(margin)";
}

base::string16 UnparsePadding(const CssPadding& padding) {
  if (!padding.HasValue())
    return L"";
  return L"NYI(padding)";
}

base::string16 UnparsePercentage(const visuals::css::Percentage& value) {
  return base::StringPrintf(L"%f%%", value.value());
}

base::string16 UnparseString(const visuals::css::String& value) {
  return value.value();
}

}  // namespace dom
