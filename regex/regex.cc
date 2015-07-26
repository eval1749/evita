//////////////////////////////////////////////////////////////////////////////
//
// Regex - API implementation
// regex/IRegex.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/IRegex.cpp#2 $
//
#include "regex/precomp.h"
#include "regex/regex.h"
#include "regex/regex_node.h"

#define DEBUG_REGEX 0

namespace Regex {

namespace RegexPrivate {

Tree* ParseRegex(IEnvironment*, LocalHeap*, const char16*, int, int);

RegexObj* CompileRegex(Regex::ICompileContext*, LocalHeap*, Tree*);

class Environment : public Regex::IEnvironment {};

char16 CharDowncase(char16 wch) {
  return reinterpret_cast<char16>(::CharLower(reinterpret_cast<LPWSTR>(wch)));
}

char16 CharUpcase(char16 wch) {
  return reinterpret_cast<char16>(::CharUpper(reinterpret_cast<LPWSTR>(wch)));
}

bool IsBothCase(char16 wch) {
  return CharUpcase(wch) != CharDowncase(wch);
}

}  // namespace RegexPrivate

using RegexPrivate::CharDowncase;
using RegexPrivate::CharUpcase;
using RegexPrivate::CompileRegex;
using RegexPrivate::Environment;
using RegexPrivate::IsBothCase;
using RegexPrivate::LocalHeap;
using RegexPrivate::ParseRegex;
using RegexPrivate::RegexObj;
using RegexPrivate::Tree;

// IEnvironment::CharDowncase
char16 IEnvironment::CharDowncase(char16 wch) const {
  return RegexPrivate::CharDowncase(wch);
}

// IEnvironment::CharUpcase
char16 IEnvironment::CharUpcase(char16 wch) const {
  return RegexPrivate::CharUpcase(wch);
}

// IEnvironment::IsBothCase
bool IEnvironment::IsBothCase(char16 wch) const {
  return RegexPrivate::IsBothCase(wch);
}

IRegex* Compile(ICompileContext* pIContext,
                const char16* pwch,
                int cwch,
                int rgfModifier) {
  LocalHeap oHeap;

  Tree* pTree = ParseRegex(pIContext, &oHeap, pwch, cwch, rgfModifier);

  if (!pTree) {
    // Maybe not enough memory.
    pIContext->SetError(0, Error_NotEnoughMemory);
    return nullptr;
  }

  if (pTree->m_iErrorCode) {
    pIContext->SetError(pTree->m_lErrorPosn, pTree->m_iErrorCode);

#if DEBUG_REGEX
    {
      const char16* pwchStart = pwch;
      const char16* pwchEnd = pwch + cwch;
      for (const char16* pwch = pwchStart; pwch < pwchEnd; pwch++) {
        if (*pwch < 0x20 || *pwch > 0x7E) {
          printf("\\u%04X", *pwch);
        } else {
          printf("%c", *pwch);
        }
      }
      printf("\n");

      pwchEnd = pwchStart + pTree->m_lErrorPosn - 1;
      for (const char16* pwch = pwchStart; pwch < pwchEnd; pwch++) {
        if (*pwch < 0x20 || *pwch > 0x7E) {
          printf("      ", *pwch);
        } else {
          printf(" ");
        }
      }
      printf("^\n");
    }
#endif
    return NULL;
  }

#if DEBUG_REGEX
  pTree->m_pNode->Print();
  printf("\n\n");
#endif

  RegexObj* pRegex = CompileRegex(pIContext, &oHeap, pTree);
  return reinterpret_cast<IRegex*>(pRegex);
}

}  /// namespace Regex
