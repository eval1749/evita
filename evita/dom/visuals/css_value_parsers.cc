// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <cmath>
#include <limits>
#include <sstream>

#include "evita/dom/visuals/css_value_parsers.h"

#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "evita/visuals/css/color.h"
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

int DoubleHex(int value) {
  DCHECK_GE(value, 0);
  DCHECK_LE(value, 15);
  return value * 16 + value;
}

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

Maybe<CssColor> ParseColor(const base::StringPiece16& text) {
  if (text.size() == 0)
    return common::Nothing<CssColor>();
  if (text[0] == '#') {
    if (text.size() == 4) {
      // #rgb
      const auto& maybe_red = ParseHex(text.substr(1, 1));
      const auto& maybe_green = ParseHex(text.substr(2, 1));
      const auto& maybe_blue = ParseHex(text.substr(3, 1));
      if (maybe_red.IsNothing() || maybe_green.IsNothing() ||
          maybe_blue.IsNothing()) {
        return common::Nothing<CssColor>();
      }
      auto const red = DoubleHex(maybe_red.FromJust());
      auto const green = DoubleHex(maybe_green.FromJust());
      auto const blue = DoubleHex(maybe_blue.FromJust());
      return common::Just<CssColor>(CssColor::Rgba(red, green, blue));
    }
    if (text.size() == 7) {
      // #rrggbb
      const auto& maybe_red = ParseHex(text.substr(1, 2));
      const auto& maybe_green = ParseHex(text.substr(3, 2));
      const auto& maybe_blue = ParseHex(text.substr(5, 2));
      if (maybe_red.IsNothing() || maybe_green.IsNothing() ||
          maybe_blue.IsNothing()) {
        return common::Nothing<CssColor>();
      }
      auto const red = maybe_red.FromJust();
      auto const green = maybe_green.FromJust();
      auto const blue = maybe_blue.FromJust();
      return common::Just<CssColor>(CssColor::Rgba(red, green, blue));
    }
  }
  if (text == L"transparent")
    return common::Just<CssColor>(CssColor());
  // TODO(eval1749): Parse color name
  // TODO(eval1749): Parse 'rgba(red, green, blue, alpha)
  return common::Nothing<CssColor>();
}

Maybe<CssLength> ParseLength(const base::StringPiece16& text) {
  enum class State {
    AfterDecimalPoint,
    DecimalPoint,
    Digit,
    SignOrDigit,
  } state = State::SignOrDigit;
  auto sign = 1.0f;
  uint64_t u64 = 0;
  auto exponent = 0;
  for (const auto code : text) {
    switch (state) {
      case State::SignOrDigit:
        if (code == '+') {
          state = State::Digit;
          break;
        }
        if (code == '-') {
          sign = -1.0f;
          state = State::Digit;
          break;
        }
        if (base::IsAsciiDigit(code)) {
          u64 = code - '0';
          state = State::Digit;
          break;
        }
        return common::Nothing<CssLength>();
      case State::Digit:
        if (base::IsAsciiDigit(code)) {
          if (u64 > std::numeric_limits<uint64_t>::max() / 10)
            return common::Nothing<CssLength>();
          u64 *= 10;
          u64 += code - '0';
          break;
        }
        if (code == '.') {
          state = State::DecimalPoint;
          break;
        }
        return common::Nothing<CssLength>();
      case State::DecimalPoint:
        if (!base::IsAsciiDigit(code))
          return common::Nothing<CssLength>();
        if (u64 > std::numeric_limits<uint64_t>::max() / 10)
          return common::Nothing<CssLength>();
        --exponent;
        u64 *= 10;
        u64 += code - '0';
        state = State::AfterDecimalPoint;
        break;
      case State::AfterDecimalPoint:
        if (!base::IsAsciiDigit(code))
          return common::Nothing<CssLength>();
        if (u64 > std::numeric_limits<uint64_t>::max() / 10)
          return common::Nothing<CssLength>();
        --exponent;
        u64 *= 10;
        u64 += code - '0';
        break;
      default:
        NOTREACHED() << "Invalid state " << static_cast<int>(state);
        return common::Nothing<CssLength>();
    }
  }
  if (exponent >= 0) {
    const auto f32 = static_cast<float>(u64) * std::pow(10.0f, exponent);
    return common::Just<CssLength>(CssLength(sign * f32));
  }
  const auto f32 = static_cast<float>(u64) / std::pow(10.0f, -exponent);
  return common::Just<CssLength>(CssLength(sign * f32));
}

Maybe<CssPercentage> ParsePercentage(const base::StringPiece16& text) {
  return common::Nothing<CssPercentage>();
}

Maybe<CssString> ParseString(const base::StringPiece16& text) {
  return common::Just<CssString>(CssString(text));
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

base::string16 UnparsePercentage(const visuals::css::Percentage& value) {
  return base::StringPrintf(L"%f%%", value.value());
}

base::string16 UnparseString(const visuals::css::String& value) {
  return value.value();
}

}  // namespace dom
