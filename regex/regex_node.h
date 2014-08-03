//////////////////////////////////////////////////////////////////////////////
//
// Regex - Parse Tree Node
// regex_node.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_node.h#6 $
//
#if !defined(INCLUDE_regex_node_h)
#define INCLUDE_regex_node_h

#include <algorithm>

#include "base/logging.h"
#include "./regex_bytecode.h"
#include "./regex_util.h"

#if !defined(DEBUG_REGEX)
#define DEBUG_REGEX 0
#endif

namespace Regex {

namespace RegexPrivate {

class Compiler;

/// <remark>
/// Regex length information
/// </remark>
struct LengthInfo {
  bool m_fFixed;
  int m_iMax;
  int m_iMin;

  /// <summary>
  /// Returns fixed match information.
  /// </summary>
  /// <returns>Value of m_fFixed</returns>
  bool IsFixed() const { return m_fFixed; }

  /// <summary>
  /// Constructs LengthInfo
  /// </summary>
  /// <param name="fFixed">
  /// True if regex matches fixed length of source
  /// </param>
  /// <param name="iMax">
  /// Maximum number of characters to be matched.
  /// </param>
  /// <param name="iMin">
  /// Minimum number of characters to be matched.
  /// </param>
  LengthInfo(bool fFixed, int iMin = 0, int iMax = 0) :
      m_fFixed(fFixed),
      m_iMax(iMax),
      m_iMin(iMin) {}
};

/// <remark>
/// For specifiying min-max parameter.
/// </remark>
struct MinMax {
  int m_iMin;
  int m_iMax;

  MinMax(int iMin = -1, int iMax = -1)
      : m_iMin(iMin), m_iMax(iMax) {}

  bool IsOne() const { return m_iMin == 1 && m_iMax == 1; }
};

/// <remark>
/// Base class of parse tree node
/// </remark>
class Node
    : public Castable_<Node>,
      public DoubleLinkedItem_<Node>,
      public LocalObject {
  public: enum Case {
    CaseSensitive = 0,
    CaseInsensitive = 1,
  };

  public: enum Direction {
    Forward = 0,
    Backward = 1,
  };

  // [C]
  public: virtual void Compile(Compiler*, int) = 0;

  public: virtual void CompileNot(Compiler*, int) { NOTREACHED(); }

  public: virtual LengthInfo ComputeLength() const {
    return LengthInfo(false);
  }

  public: virtual int ComputeMinLength() const { return 0; }

  public: virtual bool IsCharSetMember(IEnvironment*, char16) const {
    return false;
  }

  // [N]
  public: virtual bool NeedStack() const { return false; }

  public: virtual Node* Not() { NOTREACHED(); return nullptr; }

  // [R]
  public: virtual Node* Reverse() { return this; }

  // [S]
  public: virtual Node* Simplify(IEnvironment*, LocalHeap*) { return this; }

  #if DEBUG_REGEX
      public: virtual void Print() const {
        printf("%s", GetKind());
      }
  #endif
};

typedef DoubleLinkedList_<Node> Nodes;

//////////////////////////////////////////////////////////////////////
//
/// WithCase
//
class WithCase {
  private: Node::Case case_sensivity_;

  protected: WithCase(Node::Case case_sensivity)
      : case_sensivity_(case_sensivity) {}

  public: Node::Case case_sensivity() const { return case_sensivity_; }

  public: void set_case_sensivity(Node::Case case_sensivity) {
    case_sensivity_ = case_sensivity;
  }

  // [G]
  public: Node::Case GetCase() const { return case_sensivity_; }

  // [I]
  public: bool IsIgnoreCase() const {
    return Node::CaseInsensitive == case_sensivity_;
  }
};

//////////////////////////////////////////////////////////////////////
//
/// WithDirection
//
class WithDirection {
  private: Node::Direction const direction_;

  // ctor
  protected: WithDirection(Node::Direction const direction)
      : direction_(direction) {}

  // [G]
  public: Node::Direction GetDirection() const { return direction_; }

  // [I]
  public: bool IsBackward() const { return Node::Backward == direction_; }

  DISALLOW_COPY_AND_ASSIGN(WithDirection);
};

#define CASTABLE(self, base) \
  public: static const char* Kind_() { return #self; } \
  public: virtual const char* GetKind() const override { return Kind_(); } \
  public: virtual bool Is_(const char* name) const override { \
    return self::GetKind() == name || base::Is_(name); \
  }

#define CASTABLE_FINAL(self, base) \
  public: static const char* Kind_() { return #self; } \
  public: virtual const char* GetKind() const override final { \
    return Kind_(); \
  } \
  public: virtual bool Is_(const char* name) const override final { \
    return self::GetKind() == name; \
  }

//////////////////////////////////////////////////////////////////////
//
// NodeEqBase
//
class NodeEqBase : public Node, public WithDirection {
  CASTABLE(NodeEqBase, Node);

  private: bool not_;

  protected: NodeEqBase(Direction const eDirection, bool const fNot)
    : WithDirection(eDirection),
      not_(fNot) {}

  // [I]
  public: bool IsNot() const { return not_; }

  // [N]
  public: Node* Not() {
    not_ = ! not_;
    return this;
  }

  DISALLOW_COPY_AND_ASSIGN(NodeEqBase);
};

/// <remark>
/// Base class for capture reference.
/// </remark>
class NodeCaptureBase : public Node, public WithDirection {
  CASTABLE(NodeCaptureBase, Node);

  private: int nth_;

  protected: NodeCaptureBase(Direction const eDirection, int const nth)
      : WithDirection(eDirection),
        nth_(nth) {}

  // [G]
  public: int GetNth() const { return nth_; }

  // [S]
  public: void SetNth(int nth) { ASSERT(nth_ <= 0); nth_ = nth; }
};

//////////////////////////////////////////////////////////////////////
//
// NodeCsBase
//
class NodeCsBase : public NodeEqBase, public WithCase {
   CASTABLE(NodeCsBase, NodeEqBase);

  protected: NodeCsBase(
      Direction const eDirection,
      Case const case_sensivity,
      bool const not)
      : NodeEqBase(eDirection, not), WithCase(case_sensivity) {}

  DISALLOW_COPY_AND_ASSIGN(NodeCsBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeOpBase
//
class NodeOpBase : public Node {
  CASTABLE(NodeOpBase, Node);

  private: Op opcode_;

  protected: NodeOpBase(Op const opcode) : opcode_(opcode) {}

  // [C]
  public: void Compile(Compiler*, int) override;

  // [G]
  public: Op GetOp() const { return opcode_; }

  // [S]
  protected: void setOp(Op const opcode) {
    opcode_ = opcode;
  }

  DISALLOW_COPY_AND_ASSIGN(NodeOpBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeSubNodeBase
//
class NodeSubNodeBase : public Node {
  CASTABLE(NodeSubNodeBase, Node);

  private: Node* node_;

  protected: NodeSubNodeBase(Node* const pNode) : node_(pNode) {}

  // [C]
  public: int ComputeMinLength() const override {
    return node_->ComputeMinLength();
  }

  // [G]
  public: Node* GetNode() const { return node_; }

  // [N]
  public: bool NeedStack() const override { return node_->NeedStack(); }

  // [R]
  public: Node* Reverse() override {
    node_ = node_->Reverse();
    return this;
  }

  #if DEBUG_REGEX
    public: void Print() const override {
      printf("(%s ", GetKind());
      node_->Print();
      printf(")");
    }
  #endif
};

//////////////////////////////////////////////////////////////////////
//
// NodeSubNodesBase
//
class NodeSubNodesBase : public Node {
  CASTABLE(NodeSubNodesBase, Node);

  protected: Nodes m_oNodes;

  // [A]
  public: void Append(Node* pNode) { m_oNodes.Append(pNode); }

  // [D]
  public: void Delete(Node* pNode) { m_oNodes.Delete(pNode); }

  // [G]
  public: Node* GetFirst() const { return m_oNodes.GetFirst(); }

  // [R]
  public: Node* Reverse() override;

  #if DEBUG_REGEX
    public: void Print() const override {
      printf("(%s", GetKind());
      foreach (Nodes::Enum, oEnum, &m_oNodes) {
        printf(" ");
        oEnum.Get()->Print();
      }
      printf(")");
    }
  #endif
};

/// <remark>
/// Parse tree node "And".
/// </remark>
class NodeAnd : public NodeSubNodesBase {
  CASTABLE_FINAL(NodeAnd, NodeSubNodesBase);

  /// <remark>
  /// Constructs empty And node.
  /// </remark>
  public: NodeAnd() {}

  /// <remark>
  /// Constructs And node with one sub node.
  /// </remark>
  public: NodeAnd(Node* pNode1) { Append(pNode1); }

  /// <remark>
  /// Constructs And node with two sub nodes.
  /// </remark>
  public: NodeAnd(Node* pNode1, Node* pNode2) {
    Append(pNode1);
    Append(pNode2);
  }

  /// <remark>
  /// Constructs And node with three sub nodes.
  /// </remark>
  public: NodeAnd(Node* pNode1, Node* pNode2, Node* pNode3) {
    Append(pNode1);
    Append(pNode2);
    Append(pNode3);
  }

  // [C]
  /// <summary>
  /// Sum of value of all subnodes.
  /// </summary>
  public: int ComputeMinLength() const override final {
    int nMinLen = 0;
    foreach (Nodes::Enum, oEnum, &m_oNodes) {
      auto const pNode = oEnum.Get();
      nMinLen += pNode->ComputeMinLength();
      if (nMinLen >= Infinity) return Infinity;
    }
    return nMinLen;
  }

  public: void Compile(Compiler*, int) override final;

  // [S]
  public: Node* Simplify(IEnvironment*, LocalHeap*) override final;
};

/// <remark>
/// Parse tree node for dot(.)
/// </remark>
class NodeAny : public Node, public WithDirection {
  CASTABLE_FINAL(NodeAny, Node);

  // ctor
  public: NodeAny(Direction const direction) : WithDirection(direction) {}

  // [C]
  public: void Compile(Compiler*, int) override final;
  public: int ComputeMinLength() const override final { return 1; }

  // [G]
  public: Op GetOp() const { return IsBackward() ? Op_Any_B : Op_Any_F; }
};

/// <remark>
/// Parse tree node for atomic group <c>(?>...)</c>
/// </remark>
class NodeAtom : public NodeSubNodeBase {
  CASTABLE(NodeAtom, NodeSubNodeBase);

  public: NodeAtom(Node* node) : NodeSubNodeBase(node) {}

  // [C]
  public: void Compile(Compiler*, int) override;
};

/// <remark>
/// Parse tree node capture. Regex syntax is <c>(...)</c>.
/// </remark>
class NodeCapture : public NodeSubNodeBase, public WithDirection {
  CASTABLE(NodeCapture, NodeSubNodeBase);

  /// <summary>
  /// An index number of this capture.
  /// </summary>
  private: int nth_;

  /// <summary>
  /// Construct NodeCapture object.
  /// </summary>
  /// <param name="direction">A direction of capturing</param>
  /// <param name="node">A node will be captured</param>
  /// <param name="nth">Capture index number</param>
  public: NodeCapture(Direction direction, Node* node, int nth)
      : NodeSubNodeBase(node),
        WithDirection(direction),
        nth_(nth) {
    ASSERT(nth_ >= 1);
  }

  // [C]
  public: void Compile(Compiler*, int) override;

  // [G]
  /// <summary>
  /// Opcode of this node.
  /// </summary>
  /// <returns>
  /// Op_Capture_B for backward capturing, Op_Capture_F for
  /// forward capturing.
  /// </returns>
  public: Op GetOp() const {
    return IsBackward() ? Op_Capture_B : Op_Capture_F;
  }

  // [N]
  /// <summary>
  /// Alwasy true for NodeCapture.
  /// </summary>
  public: bool NeedStack() const override { return true; }

  #if DEBUG_REGEX
    public: virtual void Print() const override {
      printf("(%s[%d] ",
          IsBackward() ? "CaptureB" : "CaptureF",
          nth_);
      GetNode()->Print();
      printf(")");
    }
  #endif
};

//////////////////////////////////////////////////////////////////////
//
// NodeCaptureEq -- \<n> \k<name>
//
class NodeCaptureEq : public NodeCaptureBase, public WithCase {
  CASTABLE_FINAL(NodeCaptureEq, NodeCaptureBase);

  public: NodeCaptureEq(
      Direction const direction,
      int const nth,
      Case const case_sensivity)
        : NodeCaptureBase(direction, nth),
          WithCase(case_sensivity) {}

  // [C]
  public: void Compile(Compiler*, int) override final;

  // [G]
  public: Op GetOp() const {
    return IsBackward() ?
        IsIgnoreCase() ? Op_CaptureEq_Ci_B : Op_CaptureEq_Cs_B :
        IsIgnoreCase() ? Op_CaptureEq_Ci_F : Op_CaptureEq_Cs_F;
  }
};

//////////////////////////////////////////////////////////////////////
//
// NodeCaptureIfNot
//
class NodeCaptureIfNot : public NodeCaptureBase {
  CASTABLE_FINAL(NodeCaptureIfNot, NodeCaptureBase);

  public: NodeCaptureIfNot(Direction const direction, int const nth)
      : NodeCaptureBase(direction, nth) {}

  // [C]
  public: void Compile(Compiler*, int) override final { NOTREACHED(); }
};

/// <remark>
/// Parse tree node for character comparison.
/// </remark>
class NodeChar : public NodeCsBase {
  CASTABLE_FINAL(NodeChar, NodeCsBase);

  private: char16 char_;

  /// <summary>
  /// Construct NodeChar.
  /// </summary>
  public: NodeChar(
      Direction const direction,
      char16 char_code,
      Case const case_sensivity = CaseSensitive,
      bool not = false)
      : NodeCsBase(direction, case_sensivity, not),
        char_(char_code) {}

  // [C]
  public: void Compile(Compiler*, int) override final;
  public: void CompileNot(Compiler*, int) override final;
  public: int ComputeMinLength() const override final { return 1; }

  // [G]
  public: char16 GetChar() const { return char_; }

  public: Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
      Op_CharEq_Cs_F,
      Op_CharEq_Cs_B,

      Op_CharNe_Cs_F,
      Op_CharNe_Cs_B,

      Op_CharEq_Ci_F,
      Op_CharEq_Ci_B,

      Op_CharNe_Ci_F,
      Op_CharNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward()) k |= 1;
    if (fNot) k |= 2;
    if (IsIgnoreCase()) k |= 4;
    return k_rgeOp[k];
  }

  // [I]
  public: bool IsCharSetMember(IEnvironment*, char16) const override final;

  // [S]
  public: Node* Simplify(IEnvironment*, LocalHeap*) override final;

  #if DEBUG_REGEX
    public: virtual void Print() const override final {
      printf("(Char%s%s%s '%c')",
          IsNot() ? "Ne" : "Eq",
          IsIgnoreCase() ? "Ci" : "Cs",
          IsBackward() ? "B" : "F",
          GetChar());
    }
  #endif
};

//////////////////////////////////////////////////////////////////////
//
// NodeCharClass
//
class NodeCharClass : public NodeSubNodesBase, public WithDirection {
  CASTABLE_FINAL(NodeCharClass, NodeSubNodesBase);

  private: bool const not_;

  public: NodeCharClass(Direction const direction, bool const not)
      : WithDirection(direction),
        not_(not) {}

  // [C]
  public: void Compile(Compiler*, int) override final;
  public: int ComputeMinLength() const override final { return 1; }

  // [I]
  public: bool IsCharSetMember(IEnvironment* pIEnv, char16 wch) const {
    foreach (Nodes::Enum, oEnum, &m_oNodes) {
      if (oEnum.Get()->IsCharSetMember(pIEnv, wch)) {
        return true;
      }
    }
    return false;
  }

  public: bool IsNot() const { return not_; }

  // [N]
  public: bool NeedStack() const override final { return true; }

  // [S]
  public: Node* Simplify(IEnvironment*, LocalHeap*) override final;

  #if DEBUG_REGEX
    public: void Print() const override final {
      printf("(CharClass%s", IsNot() ? "Not" : "");
      foreach (Nodes::Enum, oEnum, &m_oNodes) {
        printf(" ");
        oEnum.Get()->Print();
      }
      printf(")");
    }
  #endif

  DISALLOW_COPY_AND_ASSIGN(NodeCharClass);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCharSet
//
class NodeCharSet : public NodeEqBase {
  CASTABLE_FINAL(NodeCharSet, NodeEqBase)

  private: int m_cwch;
  private: char16* m_pwch;

  // ctor
  public: NodeCharSet(
      Direction const direction,
      char16* const pwch,
      int const cwch,
      bool const not)
      : NodeEqBase(direction, not),
        m_cwch(cwch),
        m_pwch(pwch) {}

  // [C]
  public: void Compile(Compiler*, int) override final;
  public: int ComputeMinLength() const override final { return 1; }

  // [G]
  public: Op GetOp() const {
    return IsBackward() ?
        IsNot() ? Op_CharSetNe_B : Op_CharSetEq_B :
        IsNot() ? Op_CharSetNe_F : Op_CharSetEq_F;
  }

  public: int GetLength() const { return m_cwch; }
  public: const char16* GetString() const { return m_pwch; }

  // [I]
  public: bool IsCharSetMember(IEnvironment*, char16 wch)
      const override final {
    const char16* pwchEnd = m_pwch + m_cwch;
    for (const char16* pwch = m_pwch; pwch < pwchEnd; pwch++) {
      if (*pwch == wch) return !IsNot();
    }
    return IsNot();
  }

  #if DEBUG_REGEX
    public: void Print() const override final {
      printf("(%s '%ls')",
          IsNot() ? "CharSetNe" : "CharSetEq",
          m_pwch);
    }
  #endif
};

//////////////////////////////////////////////////////////////////////
//
// NodeIf
//
class NodeIf : public Node {
  CASTABLE_FINAL(NodeIf, Node)

  private: Node* m_pCond;
  private: Node* m_pElse;
  private: Node* m_pThen;

  // ctor
  public: NodeIf(Node* pCond, Node* pThen, Node* pElse)
    : m_pCond(pCond),
      m_pThen(pThen),
      m_pElse(pElse) {}

  // [C]
  public: void Compile(Compiler*, int) override final;
  public: int ComputeMinLength() const override final;

  // [N]
  public: bool NeedStack() const override final { return true; }

  // [R]
  public: Node* Reverse() override {
    m_pCond = m_pCond->Reverse();
    m_pThen = m_pThen->Reverse();
    m_pElse = m_pElse->Reverse();
    return this;
  }

  // [S]
  public: virtual Node* Simplify(
      IEnvironment* pIEnv,
      LocalHeap* pHeap) override {
    m_pCond = m_pCond->Simplify(pIEnv, pHeap);
    m_pThen = m_pThen->Simplify(pIEnv, pHeap);
    m_pElse = m_pElse->Simplify(pIEnv, pHeap);
    return this;
  }

  #if DEBUG_REGEX
    public: virtual void Print() const override final {
      printf("(if ");
      m_pCond->Print();
      printf(" ");
      m_pThen->Print();
      printf(" ");
      m_pElse->Print();
      printf(")");
    }
  #endif
};

//////////////////////////////////////////////////////////////////////
//
// NodeLookaround
//
class NodeLookaround : public NodeSubNodeBase {
  CASTABLE_FINAL(NodeLookaround, NodeSubNodeBase);

  private: bool const m_fPositive;

  public: NodeLookaround(Node* pNode, bool fPositive)
      : NodeSubNodeBase(pNode),
        m_fPositive(fPositive) {}

  // [C]
  public: virtual void Compile(Compiler*, int) override final;
  public: virtual int ComputeMinLength() const override final { return 0; }

  // [I]
  public: bool IsPositive() const { return m_fPositive; }

  // [N]
  public: virtual bool NeedStack() const override final { return true; }

  DISALLOW_COPY_AND_ASSIGN(NodeLookaround);
};

//////////////////////////////////////////////////////////////////////
//
// NodeMinMax
// Base class for NodeMax and NodeMin.
//
class NodeMinMax : public NodeSubNodeBase, public WithDirection {
  CASTABLE(NodeMinMax, NodeSubNodeBase);

  private: int const m_iMax;
  private: int const m_iMin;

  protected: NodeMinMax(
      Direction const direction,
      Node* const node,
      MinMax minmax)
      : NodeSubNodeBase(node),
        WithDirection(direction),
        m_iMax(minmax.m_iMax),
        m_iMin(minmax.m_iMin) {}

  // [C]
  public: virtual int ComputeMinLength() const override {
    return GetNode()->ComputeMinLength() * m_iMin;
  }

  // [G]
  public: int GetMax() const { return m_iMax; }
  public: int GetMin() const { return m_iMin; }

  // [N]
  public: virtual bool NeedStack() const override { return true; }

  #if DEBUG_REGEX
    public: virtual void Print() const override {
      printf("(%s min=%d max=%d ", GetKind(), GetMin(), GetMax());
          GetNode()->Print();
          printf(")");
    }
  #endif

  DISALLOW_COPY_AND_ASSIGN(NodeMinMax);
};

//////////////////////////////////////////////////////////////////////
//
// NodeMax
//
class NodeMax : public NodeMinMax {
  CASTABLE_FINAL(NodeMax, NodeMinMax);

  public: NodeMax(Direction direction, Node* node, MinMax minmax)
      : NodeMinMax(direction, node, minmax) {}

  // [C]
  public: virtual void Compile(Compiler*, int) override final;
};

//////////////////////////////////////////////////////////////////////
//
// NodeMin
//
class NodeMin : public NodeMinMax {
  CASTABLE_FINAL(NodeMin, NodeMinMax);

  public: NodeMin(Direction direction, Node* node, MinMax minmax)
      : NodeMinMax(direction, node, minmax) {}

  // [C]
  public: virtual void Compile(Compiler*, int) override final;
};

//////////////////////////////////////////////////////////////////////
//
// NodeOneWidth
//
class NodeOneWidth : public NodeOpBase {
  CASTABLE_FINAL(NodeOneWidth, NodeOpBase);

  public: NodeOneWidth(Op opcode) : NodeOpBase(opcode) {}

  // [C]
  public: virtual void CompileNot(Compiler*, int) override final;
  public: virtual int ComputeMinLength() const override final { return 1; }

  // [G]
  public: Op GetNotOp() const { return static_cast<Op>(GetOp() + 1); }

  // [I]
  public: virtual bool IsCharSetMember(IEnvironment*, char16 wch)
    const override final;

  // [N]
  public: virtual Node* Not() override final;

};

//////////////////////////////////////////////////////////////////////
//
// NodeOr
//
class NodeOr : public NodeSubNodesBase {
  CASTABLE_FINAL(NodeOr, NodeSubNodesBase);

  // ctor
  public: NodeOr() {}

  public: NodeOr(Node* pNode1) { m_oNodes.Append(pNode1); }

  public: NodeOr(Node* pNode1, Node* pNode2) {
      m_oNodes.Append(pNode1);
      m_oNodes.Append(pNode2);
  }

  // [C]
  // ComputeMinLength - minimum value of all subnodes.
  public: virtual int ComputeMinLength() const override final {
    auto nMinLen = int(Infinity);
    foreach (Nodes::Enum, oEnum, &m_oNodes) {
        auto const pNode = oEnum.Get();
        nMinLen = std::min(nMinLen, pNode->ComputeMinLength());
    }
    return nMinLen;
  }

  public: virtual void Compile(Compiler*, int) override final;

  // [G]
  public: Nodes* GetNodes() { return &m_oNodes; }

  // [N]
  public: virtual bool NeedStack() const override final { return true; }
};

//////////////////////////////////////////////////////////////////////
//
// NodeRange
// Represent simple character class, e.g. [A-Z].
//
class NodeRange : public NodeCsBase {
  CASTABLE_FINAL(NodeRange, NodeCsBase);

  private: char16 const m_wchMin;
  private: char16 const m_wchMax;

  public: NodeRange(
      Direction direction,
      char16 wchMin,
      char16 wchMax,
      Case case_sensivity,
      bool not = false)
      : NodeCsBase(direction, case_sensivity, not),
        m_wchMin(wchMin),
        m_wchMax(wchMax) {}

  // [C]
  public: virtual void Compile(Compiler*, int) override final;
  public: virtual void CompileNot(Compiler*, int) override final;
  public: virtual int ComputeMinLength() const override final { return 1; }

  // [G]
  public: char16 GetMaxChar() const { return m_wchMax; }
  public: char16 GetMinChar() const { return m_wchMin; }

  public: Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
      Op_RangeEq_Cs_F,
      Op_RangeEq_Cs_B,

      Op_RangeNe_Cs_F,
      Op_RangeNe_Cs_B,

      Op_RangeEq_Ci_F,
      Op_RangeEq_Ci_B,

      Op_RangeNe_Ci_F,
      Op_RangeNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward()) k |= 1;
    if (fNot) k |= 2;
    if (IsIgnoreCase()) k |= 4;
    return k_rgeOp[k];
  }

  // [I]
  public: virtual bool IsCharSetMember(IEnvironment*, char16)
    const override final;

  // [S]
  public: virtual Node* Simplify(IEnvironment*, LocalHeap*) override final;

  #if DEBUG_REGEX
    public: virtual void Print() const override {
      printf("(Range%s%s %lc-%lc)",
          IsNot() ? "Not" : "",
          IsIgnoreCase() ? "Ci" : "Cs",
          GetMinChar(),
          GetMaxChar());
    }
  #endif

  DISALLOW_COPY_AND_ASSIGN(NodeRange);
};

/// <remark>
/// Parse tree node for string comparison.
/// </remark>
class NodeString : public NodeCsBase {
  CASTABLE_FINAL(NodeString, NodeCsBase);

  private: int const m_cwch;
  private: const char16* const m_pwch;

  /// <summary>
  /// Construct NodeString.
  /// </summary>
  public: NodeString(
      Direction direction,
      const char16* pwch,
      int cwch,
      Case case_sensivity = CaseSensitive,
      bool not = false);

  public: virtual ~NodeString();

  // [C]
  public: virtual int ComputeMinLength() const override final {
    return m_cwch;
  }

  public: virtual void Compile(Compiler*, int) override final;

  // [G]
  public: int GetLength() const { return m_cwch; }

  public: Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
      Op_StringEq_Cs_F,
      Op_StringEq_Cs_B,

      Op_StringNe_Cs_F,
      Op_StringNe_Cs_B,

      Op_StringEq_Ci_F,
      Op_StringEq_Ci_B,

      Op_StringNe_Ci_F,
      Op_StringNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward()) k |= 1;
    if (fNot) k |= 2;
    if (IsIgnoreCase()) k |= 4;
    return k_rgeOp[k];
  }

  public: const char16* GetStart() const { return m_pwch; }

  // [P]
  #if DEBUG_REGEX
    public: virtual void Print() const override final {
      printf("(String%s%s%s '%ls')",
          IsNot() ? "Ne" : "Eq",
          IsIgnoreCase() ? "Ci" : "Cs",
          IsBackward() ? "B" : "F",
          m_pwch);
      }
  #endif

  DISALLOW_COPY_AND_ASSIGN(NodeString);
};

/// <summary>
/// Parse tree node for void. This is used for empty capture.
/// </summary>
class NodeVoid : public Node {
  CASTABLE_FINAL(NodeVoid, Node);

  // [C]
  public: virtual void Compile(Compiler*, int) override {}
};

//////////////////////////////////////////////////////////////////////
//
// NodeZeroWidth
// Op_AsciiBoundary "(?a:\b)"
// Op_AfterNewline "^"m scanner
// Op_AsciiNotBoundary "(?a:\B)"
// Op_BeforeNewLine "(?m:$)" scanner
// Op_EndOfLine "$" "\Z" scanner
// Op_EndOfString "(?s:$)" "\z" scanner
// Op_Posn "\G" scanner
// Op_StartOfString "^" "\A" scanner
// Op_UnicodeBoundary "(?:u\b)"
// Op_UnicodeNotBoundary "(?:u\B)"
//
// See Also:
// k_rgoZeroWidthMap in regex_parse.cpp
// Compiler::compileScanner in regex_compile.cpp
// Engine::Execute in regex_exec.cpp
//
class NodeZeroWidth : public NodeOpBase {
  CASTABLE_FINAL(NodeZeroWidth, NodeOpBase)
  public: NodeZeroWidth(Op opcode) : NodeOpBase(opcode) {}
};

//////////////////////////////////////////////////////////////////////
//
// CaptureDef
//
struct CaptureDef
    : LocalObject,
      DoubleLinkedItem_<CaptureDef> {
  int m_iNth;
  char16* m_pwszName;

  CaptureDef(char16* pwszName, int iNth)
    : m_iNth(iNth), m_pwszName(pwszName) {}
};

typedef DoubleLinkedList_<CaptureDef> CaptureDefs;

//////////////////////////////////////////////////////////////////////
//
// Tree
//
class Tree : public LocalObject {
  public: int m_cCaptures;
  public: int m_iErrorCode;
  public: long m_lErrorPosn;
  public: CaptureDefs m_oCaptures;
  public: Node* m_pNode;
  public: int m_rgfOption;
  public: char16* m_prgpwszCaptureName;

  public: Tree(int rgfOption)
    : m_cCaptures(0),
      m_iErrorCode(0),
      m_lErrorPosn(0),
      m_pNode(nullptr),
      m_rgfOption(rgfOption),
      m_prgpwszCaptureName(nullptr) {}
};

} // RegexPrivate
} // Regex

#endif //!defined(INCLUDE_regex_node_h)
