// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/selector_parser.h"

#include "base/logging.h"
#include "evita/visuals/css/selector_builder.h"

namespace visuals {
namespace css {

namespace {
bool IsNameChar(base::char16 ch) {
  if (ch >= '0' && ch < '9')
    return true;
  if (ch >= 'A' && ch < 'Z')
    return true;
  if (ch >= 'a' && ch < 'z')
    return true;
  return ch == '-' || ch == '_';
}
}  // namespace

using Parser = Selector::Parser;

Parser::Parser() = default;
Parser::~Parser() = default;

Selector Parser::Error(base::StringPiece16 error, size_t position) {
  DCHECK(error_.empty()) << error_.as_string();
  error_ = error;
  position_ = position;
  return Selector();
}

Selector Parser::Parse(base::StringPiece16 text) {
  Builder builder;
  enum {
    kClass,
    kColon,
    kId,
    kTagName,
    kStart,
  } state = kStart;
  size_t start = 0;
  for (size_t index = 0; index < text.size(); ++index) {
    const auto ch = text[index];
    const auto length = index - start;
    switch (state) {
      case kClass:
        if (ch == '.' || ch == ':') {
          if (length == 0)
            return Error(L"Empty class", index);
          if (text[start] == ':' && length == 1)
            return Error(L"Empty pseudo-class", index);
          builder.AddClass(text.substr(start, length));
          if (ch == ':')
            start = index;
          else
            start = index + 1;
          state = kClass;
          continue;
        }
        if (!IsNameChar(ch))
          return Error(L"Bad class", index);
        continue;
      case kColon:
        if (ch == ':') {
          state = kTagName;
          continue;
        }
        if (!IsNameChar(ch))
          return Error(L"Bad colon", index);
        state = kClass;
        continue;
      case kId:
        if (ch == '.' || ch == ':') {
          if (length == 0)
            return Error(L"Empty id", index);
          builder.SetId(text.substr(start, length));
          if (ch == ':')
            start = index;
          else
            start = index + 1;
          state = kClass;
          continue;
        }
        if (!IsNameChar(ch))
          return Error(L"Bad id", index);
        continue;
      case kStart:
        if (ch == ':') {
          start = index;
          state = kColon;
          continue;
        }
        if (ch == '.') {
          start = index  + 1;
          state = kClass;
          continue;
        }
        if (ch == '#') {
          start = index + 1;
          state = kId;
          continue;
        }
        if (!IsNameChar(ch))
          return Error(L"Bad tag name", index);
        start = index;
        state = kTagName;
        continue;
      case kTagName:
        if (ch == '.' || ch == ':' || ch == '#') {
          if (text[start] == ':' && length < 3)
            return Error(L"Empty pseudo-element", text.length());
          if (length == 0)
            return Error(L"Empty tag name", index);
          builder.SetTagName(text.substr(start, length));
          if (ch == ':') {
            start = index;
            state = kClass;
            continue;
          }
          if (ch == '#') {
            start = index + 1;
            state = kId;
            continue;
          }
          start = index + 1;
          state = kClass;
          continue;
        }
        if (!IsNameChar(ch))
          return Error(L"Bad tag name", index);
        continue;
    }
    NOTREACHED() << "We should handle state " << state;
  }
  const auto length = text.length() - start;
  switch (state) {
    case kClass:
      if (length == 0)
        return Error(L"Empty class", text.length());
      if (text[start] == ':' && length < 2)
        return Error(L"Empty pseudo-class", text.length());
      builder.AddClass(text.substr(start, length));
      break;
    case kColon:
      return Error(L"Empty class", text.length());
    case kId:
      if (length == 0)
        return Error(L"Empty id", text.length());
      builder.SetId(text.substr(start, length));
      break;
    case kStart:
      break;
    case kTagName:
      if (text[start] == ':' && length < 3)
        return Error(L"Empty pseudo-element", text.length());
      DCHECK_GT(length, 0);
      builder.SetTagName(text.substr(start, length));
      break;
    default:
      NOTREACHED() << "We shold handle case " << start;
      break;
  }
  return builder.Build();
}

}  // namespace css
}  // namespace visuals
