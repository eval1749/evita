//////////////////////////////////////////////////////////////////////////////
//
// Regex - Parse Tree Node
// regex/regex_node.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_node.cpp#4 $
//
#include <algorithm>

#include "regex/precomp.h"
#include "regex/regex.h"
#include "regex/regex_node.h"

namespace Regex {
namespace RegexPrivate {

//////////////////////////////////////////////////////////////////////
//
// NodeAnd::Simplify
//
Node* NodeAnd::Simplify(IEnvironment* pIEnv, LocalHeap* pHeap) {
  if (m_oNodes.IsEmpty())
    return new (pHeap) NodeVoid;
  if (NULL == m_oNodes.GetFirst()->GetNext())
    return m_oNodes.GetFirst();

  enum Case {
    Case_Unknown,

    Case_Ignore,
    Case_Sensitive,
  } eCase = Case_Unknown;

  Direction eDir = Forward;

  Nodes oChars;
  CharSink oSink(pHeap);
  Nodes::Enum oEnum(&m_oNodes);
  while (!oEnum.AtEnd()) {
    Node* pNode = oEnum.Get();
    oEnum.Next();

    pNode->Simplify(pIEnv, pHeap);

    NodeChar* pChar = pNode->DynamicCast<NodeChar>();
    if (NULL != pChar && pChar->IsNot())
      pChar = NULL;

    if (NULL != pChar) {
      eDir = pChar->GetDirection();

      switch (eCase) {
        case Case_Unknown:
          eCase = pChar->IsIgnoreCase() ? Case_Ignore : Case_Sensitive;
          break;

        case Case_Ignore:
          if (!pChar->IsIgnoreCase()) pChar = NULL;
          break;

        case Case_Sensitive:
          if (pChar->IsIgnoreCase())
            pChar = NULL;
          break;

        default:
          NOTREACHED();
      }
    }

    if (NULL != pChar) {
      m_oNodes.Delete(pChar);
      oChars.Append(pChar);
      oSink.Add(pChar->GetChar());
    } else if (oChars.IsEmpty()) {
      // nothing to do
    } else if (NULL == oChars.GetFirst()->GetNext()) {
      Node* pFirst = oChars.GetFirst();
      oChars.Delete(pFirst);

      m_oNodes.InsertBefore(pFirst, pNode);

      oSink.Reset();
      eCase = Case_Unknown;
    } else {
      NodeString* pString = new (pHeap)
          NodeString(eDir, oSink.Save(pHeap), oSink.GetLength(),
                     Case_Ignore == eCase ? CaseInsensitive : CaseSensitive);

      m_oNodes.InsertBefore(pString, pNode);

      oChars.DeleteAll();
      oSink.Reset();
      eCase = Case_Unknown;
    }
  }

  if (oChars.IsEmpty()) {
    // nothing to do
  } else if (NULL == oChars.GetFirst()->GetNext()) {
    Node* pFirst = oChars.GetFirst();
    oChars.Delete(pFirst);

    m_oNodes.Append(pFirst);
  } else {
    NodeString* pString = new (pHeap)
        NodeString(eDir, oSink.Save(pHeap), oSink.GetLength(),
                   Case_Ignore == eCase ? CaseInsensitive : CaseSensitive);

    m_oNodes.Append(pString);
  }
  return this;
}

//////////////////////////////////////////////////////////////////////
//
// NodeChar::IsCharSetMember
//
bool NodeChar::IsCharSetMember(IEnvironment* pIEnv, char16 wch) const {
  if (GetChar() == wch)
    return !IsNot();
  if (!IsIgnoreCase()) return IsNot();
  wch = pIEnv->CharUpcase(wch);
  if (GetChar() == wch)
    return !IsNot();
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// NodeChar::Simplify
//
Node* NodeChar::Simplify(IEnvironment* pIEnv, LocalHeap*) {
  if (IsIgnoreCase()) {
    set_case_sensitivity(pIEnv->IsBothCase(char_) ? CaseInsensitive
                                                  : CaseSensitive);
    if (IsIgnoreCase()) {
      char_ = pIEnv->CharUpcase(char_);
    }
  }

  return this;
}

//////////////////////////////////////////////////////////////////////
//
// NodeCharClass::Simplify
//
//  o One member
//  o CharSet
//
Node* NodeCharClass::Simplify(IEnvironment* pIEnv, LocalHeap* pHeap) {
  if (m_oNodes.IsEmpty()) {
    return new (pHeap) NodeVoid;
  }

  if (NULL == m_oNodes.GetFirst()->GetNext()) {
    Node* pNode = m_oNodes.GetFirst();
    pNode = pNode->Simplify(pIEnv, pHeap);
    if (IsNot()) {
      pNode = pNode->Not();
    }
    return pNode;
  }

  Nodes oChars;
  {
    Nodes::Enum oEnum(&m_oNodes);
    while (!oEnum.AtEnd()) {
      Node* pNode = oEnum.Get();
      oEnum.Next();
      if (pNode->Is<NodeChar>()) {
        m_oNodes.Delete(pNode);
        oChars.Append(pNode);
      }
    }
  }

  if (oChars.IsEmpty()) {
    return this;
  }

  CharSink oSink(pHeap);
  for (Nodes::Enum oEnum(&oChars); !oEnum.AtEnd(); oEnum.Next()) {
    NodeChar* pNode = oEnum.Get()->StaticCast<NodeChar>();
    char16 wch = pNode->GetChar();
    if (!IsCharSetMember(pIEnv, wch)) {
      oSink.Add(wch);
    }

    if (pNode->IsIgnoreCase()) {
      char16 wchU = pIEnv->CharUpcase(wch);
      char16 wchD = pIEnv->CharDowncase(wch);
      if (wch != wchU && !IsCharSetMember(pIEnv, wchU)) {
        oSink.Add(wch);
      }

      if (wch != wchD && !IsCharSetMember(pIEnv, wchD)) {
        oSink.Add(wch);
      }
    }
  }

  switch (oSink.GetLength()) {
    case 0:
      break;

    case 1:
      m_oNodes.Append(new (pHeap) NodeChar(GetDirection(), oSink.Get(0)));
      break;

    default: {
      NodeCharSet* pCharSet = new (pHeap) NodeCharSet(
          GetDirection(), oSink.Save(pHeap), oSink.GetLength(), IsNot());
      if (m_oNodes.IsEmpty()) {
        return pCharSet;
      }
      Append(pCharSet);
      break;
    }
  }

  return this;
}

//////////////////////////////////////////////////////////////////////
//
// NodeIf::ComputeMinLength
//
int NodeIf::ComputeMinLength() const {
  return std::min(m_pThen->ComputeMinLength(), m_pElse->ComputeMinLength());
}

//////////////////////////////////////////////////////////////////////
//
// NodeOneWidth::IsCharSetMember
//
bool NodeOneWidth::IsCharSetMember(IEnvironment* pIEnv, char16 wch) const {
#define case_Op_(mp_name)                  \
  case Op_Ascii##mp_name##Eq_B:            \
  case Op_Ascii##mp_name##Eq_F:            \
    return pIEnv->IsAscii##mp_name(wch);   \
  case Op_Ascii##mp_name##Ne_B:            \
  case Op_Ascii##mp_name##Ne_F:            \
    return !pIEnv->IsAscii##mp_name(wch);  \
  case Op_Unicode##mp_name##Eq_B:          \
  case Op_Unicode##mp_name##Eq_F:          \
    return pIEnv->IsUnicode##mp_name(wch); \
  case Op_Unicode##mp_name##Ne_B:          \
  case Op_Unicode##mp_name##Ne_F:          \
    return !pIEnv->IsUnicode##mp_name(wch);

  switch (GetOp()) {
    case_Op_(DigitChar) case_Op_(SpaceChar) case_Op_(WordChar) default
        : NOTREACHED();
    return false;
  }

#undef case_Op_
}

/// <summary>
///   Not operation.
/// </summary>
Node* NodeOneWidth::Not() {
#define case_Op_(mp_name)             \
  case Op_Ascii##mp_name##Eq_B:       \
    setOp(Op_Ascii##mp_name##Ne_B);   \
    break;                            \
  case Op_Ascii##mp_name##Eq_F:       \
    setOp(Op_Ascii##mp_name##Ne_F);   \
    break;                            \
  case Op_Ascii##mp_name##Ne_B:       \
    setOp(Op_Ascii##mp_name##Eq_B);   \
    break;                            \
  case Op_Ascii##mp_name##Ne_F:       \
    setOp(Op_Ascii##mp_name##Eq_F);   \
    break;                            \
  case Op_Unicode##mp_name##Eq_B:     \
    setOp(Op_Unicode##mp_name##Ne_B); \
    break;                            \
  case Op_Unicode##mp_name##Eq_F:     \
    setOp(Op_Unicode##mp_name##Ne_F); \
    break;                            \
  case Op_Unicode##mp_name##Ne_B:     \
    setOp(Op_Unicode##mp_name##Eq_B); \
    break;                            \
  case Op_Unicode##mp_name##Ne_F:     \
    setOp(Op_Unicode##mp_name##Eq_F); \
    break;

  switch (GetOp()) {
    case_Op_(DigitChar) case_Op_(SpaceChar) case_Op_(WordChar) default
        : NOTREACHED();
  }

#undef case_Op_

  return this;
}

//////////////////////////////////////////////////////////////////////
//
// NodeRange::IsCharSetMember
//
bool NodeRange::IsCharSetMember(IEnvironment* pIEnv, char16 wch) const {
  if (wch >= GetMinChar() && wch <= GetMaxChar())
    return !IsNot();
  if (IsIgnoreCase()) {
    wch = pIEnv->CharUpcase(wch);
    if (wch >= GetMinChar() && wch <= GetMaxChar())
      return !IsNot();
  }
  return IsNot();
}

//////////////////////////////////////////////////////////////////////
//
// NodeRange::Simplify
//
Node* NodeRange::Simplify(IEnvironment* pIEnv, LocalHeap* pHeap) {
  if (IsIgnoreCase()) {
    char16 wchMin = GetMinChar();
    char16 wchMax = GetMaxChar();

    if (pIEnv->IsBothCase(wchMin) && pIEnv->IsBothCase(wchMax)) {
      set_case_sensitivity(CaseInsensitive);
    } else {
      set_case_sensitivity(CaseSensitive);
    }

    if (IsIgnoreCase()) {
      wchMin = pIEnv->CharUpcase(wchMin);
      wchMax = pIEnv->CharUpcase(wchMax);
    }
  }

  if (GetMinChar() == GetMaxChar()) {
    return new (pHeap)
        NodeChar(GetDirection(), GetMinChar(), GetCase(), IsNot());
  }

  return this;
}

//////////////////////////////////////////////////////////////////////
//
// NodeString
//
NodeString::NodeString(Direction direction,
                       const char16* pwch,
                       int cwch,
                       Case case_sensitivity,
                       bool not)
    : NodeCsBase(direction, case_sensitivity, not),
      m_cwch(cwch),
      m_pwch(pwch) {}

//////////////////////////////////////////////////////////////////////
//
// NodeSubNodesBase::Reverse
//
Node* NodeSubNodesBase::Reverse() {
  Nodes oNodes;
  {
    Nodes::Enum oEnum(&m_oNodes);
    while (!oEnum.AtEnd()) {
      Node* pNode = oEnum.Get();
      oEnum.Next();
      m_oNodes.Delete(pNode);
      oNodes.Append(pNode->Reverse());
    }
  }

  {
    Nodes::Enum oEnum(&oNodes);
    while (!oEnum.AtEnd()) {
      Node* pNode = oEnum.Get();
      oEnum.Next();
      oNodes.Delete(pNode);
      m_oNodes.Prepend(pNode);
    }
  }

  return this;
}

}  // namespace RegexPrivate
}  // namespace Regex
