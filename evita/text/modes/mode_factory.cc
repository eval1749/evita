// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/modes/mode_factory.h"

#include "base/logging.h"
#include "evita/text/modes/char_syntax.h"
#include "evita/text/modes/mode_chooser.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// ModeFactory
//
ModeFactory::ModeFactory() {
}

ModeFactory::~ModeFactory() {
}

bool ModeFactory::IsSupported(const char16* pwszName) const {
  DCHECK(pwszName);

  const char16* pwszExt = lstrrchrW(pwszName, '.');
  if (!pwszExt) {
    // Name doesn't have no file extension.
    return false;
  }

  pwszExt++; // skip dot

  enum State {
    State_Start,

    State_Match,
    State_Skip,
    State_SkipSpace,
  } eState = State_Start;

  auto pwsz = pwszExt;
  auto pwszRunner = getExtensions();
  while (*pwszRunner) {
    char16 wch = *pwszRunner++;
    switch (eState) {
      case State_Start:
        if (!wch || IsWhitespace(wch))
          return false;

      firstChar:
        if (*pwszExt == wch) {
          pwsz = pwszExt + 1;
          eState = State_Match;
        } else {
          eState = State_Skip;
        }
        break;

      case State_Match:
        if (!wch)
          return !*pwsz;

        if (IsWhitespace(wch)) {
          if (!*pwsz)
            return true;
          
          eState = State_SkipSpace;
          break;
        }

        if (*pwsz == wch)
          pwsz++;
        else
          eState = State_Skip;
        break;

      case State_Skip:
        if (!wch)
          return false;

        if (IsWhitespace(wch))
          eState = State_SkipSpace;
        break;

      case State_SkipSpace:
        if (!wch)
          return false;

        if (!IsWhitespace(wch)) {
          pwsz = pwszExt;
          goto firstChar;
        }
        break;

      default:
        NOTREACHED();
    }
  }

  return State_Match == eState && !*pwsz;
}

}  // namespace text
