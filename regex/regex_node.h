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
#ifndef REGEX_REGEX_NODE_H_
#define REGEX_REGEX_NODE_H_

#include <algorithm>

#include "base/logging.h"
#include "regex/regex_bytecode.h"
#include "regex/regex_util.h"

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
  explicit LengthInfo(bool fFixed, int iMin = 0, int iMax = 0)
      : m_fFixed(fFixed), m_iMax(iMax), m_iMin(iMin) {}
};

/// <remark>
/// For specifiying min-max parameter.
/// </remark>
struct MinMax {
  int m_iMin;
  int m_iMax;

  explicit MinMax(int iMin = -1, int iMax = -1) : m_iMin(iMin), m_iMax(iMax) {}

  bool IsOne() const { return m_iMin == 1 && m_iMax == 1; }
};

/// <remark>
/// Base class of parse tree node
/// </remark>
class Node : public Castable_<Node>,
             public DoubleLinkedItem_<Node>,
             public LocalObject {
 public:
  enum Case {
    CaseSensitive = 0,
    CaseInsensitive = 1,
  };

  enum Direction {
    Forward = 0,
    Backward = 1,
  };

  virtual void Compile(Compiler*, int) = 0;
  virtual void CompileNot(Compiler*, int) { NOTREACHED(); }
  virtual LengthInfo ComputeLength() const { return LengthInfo(false); }
  virtual int ComputeMinLength() const { return 0; }
  virtual bool IsCharSetMember(IEnvironment*, char16) const { return false; }
  virtual bool NeedStack() const { return false; }
  virtual Node* Not() {
    NOTREACHED();
    return nullptr;
  }
  virtual Node* Reverse() { return this; }
  virtual Node* Simplify(IEnvironment*, LocalHeap*) { return this; }

#if DEBUG_REGEX
  virtual void Print() const { printf("%s", GetKind()); }
#endif

 protected:
  Node() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Node);
};

typedef DoubleLinkedList_<Node> Nodes;

//////////////////////////////////////////////////////////////////////
//
/// WithCase
//
class WithCase {
 public:
  Node::Case case_sensitivity() const { return case_sensitivity_; }
  void set_case_sensitivity(Node::Case case_sensitivity) {
    case_sensitivity_ = case_sensitivity;
  }

  Node::Case GetCase() const { return case_sensitivity_; }
  bool IsIgnoreCase() const {
    return Node::CaseInsensitive == case_sensitivity_;
  }

 protected:
  explicit WithCase(Node::Case case_sensitivity)
      : case_sensitivity_(case_sensitivity) {}

 private:
  Node::Case case_sensitivity_;

  DISALLOW_COPY_AND_ASSIGN(WithCase);
};

//////////////////////////////////////////////////////////////////////
//
/// WithDirection
//
class WithDirection {
 public:
  Node::Direction GetDirection() const { return direction_; }
  bool IsBackward() const { return Node::Backward == direction_; }

 protected:
  explicit WithDirection(Node::Direction const direction)
      : direction_(direction) {}

 private:
  Node::Direction const direction_;

  DISALLOW_COPY_AND_ASSIGN(WithDirection);
};

#define CASTABLE(self, base)                               \
 public:                                                   \
  static const char* Kind_() { return #self; }             \
  const char* GetKind() const override { return Kind_(); } \
  bool Is_(const char* name) const override {              \
    return self::GetKind() == name || base::Is_(name);     \
  }

#define CASTABLE_FINAL(self, base)                      \
 public:                                                \
  static const char* Kind_() { return #self; }          \
  const char* GetKind() const final { return Kind_(); } \
  bool Is_(const char* name) const final { return self::GetKind() == name; }

//////////////////////////////////////////////////////////////////////
//
// NodeEqBase
//
class NodeEqBase : public Node, public WithDirection {
  CASTABLE(NodeEqBase, Node);

 public:
  bool IsNot() const { return not_; }

  Node* Not() {
    not_ = !not_;
    return this;
  }

 protected:
  NodeEqBase(Direction const eDirection, bool const fNot)
      : WithDirection(eDirection), not_(fNot) {}

 private:
  bool not_;

  DISALLOW_COPY_AND_ASSIGN(NodeEqBase);
};

/// <remark>
/// Base class for capture reference.
/// </remark>
class NodeCaptureBase : public Node, public WithDirection {
  CASTABLE(NodeCaptureBase, Node);

 public:
  int GetNth() const { return nth_; }

  void SetNth(int nth) {
    DCHECK_LE(nth_, 0);
    nth_ = nth;
  }

 protected:
  NodeCaptureBase(Direction const eDirection, int const nth)
      : WithDirection(eDirection), nth_(nth) {}

 private:
  int nth_;

  DISALLOW_COPY_AND_ASSIGN(NodeCaptureBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCsBase
//
class NodeCsBase : public NodeEqBase, public WithCase {
  CASTABLE(NodeCsBase, NodeEqBase);

 protected:
  NodeCsBase(Direction const eDirection,
             Case const case_sensitivity,
             bool const not)
      : NodeEqBase(eDirection, not), WithCase(case_sensitivity) {}

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeCsBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeOpBase
//
class NodeOpBase : public Node {
  CASTABLE(NodeOpBase, Node);

 public:
  Op GetOp() const { return opcode_; }

  // Node
  void Compile(Compiler*, int) override;

 protected:
  explicit NodeOpBase(Op const opcode) : opcode_(opcode) {}

  void setOp(Op const opcode) { opcode_ = opcode; }

 private:
  Op opcode_;

  DISALLOW_COPY_AND_ASSIGN(NodeOpBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeSubNodeBase
//
class NodeSubNodeBase : public Node {
  CASTABLE(NodeSubNodeBase, Node);

  // [G]
  Node* GetNode() const { return node_; }

  // Node
  int ComputeMinLength() const override { return node_->ComputeMinLength(); }
  bool NeedStack() const override { return node_->NeedStack(); }

  Node* Reverse() override {
    node_ = node_->Reverse();
    return this;
  }

#if DEBUG_REGEX
  void Print() const override {
    printf("(%s ", GetKind());
    node_->Print();
    printf(")");
  }
#endif

 protected:
  explicit NodeSubNodeBase(Node* const pNode) : node_(pNode) {}

 private:
  Node* node_;

  DISALLOW_COPY_AND_ASSIGN(NodeSubNodeBase);
};

//////////////////////////////////////////////////////////////////////
//
// NodeSubNodesBase
//
class NodeSubNodesBase : public Node {
  CASTABLE(NodeSubNodesBase, Node);

 public:
  void Append(Node* pNode) { m_oNodes.Append(pNode); }
  void Delete(Node* pNode) { m_oNodes.Delete(pNode); }
  Node* GetFirst() const { return m_oNodes.GetFirst(); }

  // Node
  Node* Reverse() override;

#if DEBUG_REGEX
  void Print() const override {
    printf("(%s", GetKind());
    for (Nodes::Enum oEnum(&m_oNodes); !oEnum.AtEnd(); oEnum.Next()) {
      printf(" ");
      oEnum.Get()->Print();
    }
    printf(")");
  }
#endif

 protected:
  NodeSubNodesBase() = default;

  Nodes m_oNodes;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeSubNodesBase);
};

/// <remark>
/// Parse tree node "And".
/// </remark>
class NodeAnd final : public NodeSubNodesBase {
  CASTABLE_FINAL(NodeAnd, NodeSubNodesBase);

 public:
  /// <remark>
  /// Constructs And node with one sub node.
  /// </remark>
  explicit NodeAnd(Node* pNode1) { Append(pNode1); }

  /// <remark>
  /// Constructs And node with two sub nodes.
  /// </remark>
  NodeAnd(Node* pNode1, Node* pNode2) {
    Append(pNode1);
    Append(pNode2);
  }

  /// <remark>
  /// Constructs And node with three sub nodes.
  /// </remark>
  NodeAnd(Node* pNode1, Node* pNode2, Node* pNode3) {
    Append(pNode1);
    Append(pNode2);
    Append(pNode3);
  }
  NodeAnd() {}

  // Node
  /// <summary>
  /// Sum of value of all subnodes.
  /// </summary>
  int ComputeMinLength() const final {
    auto nMinLen = 0;
    for (Nodes::Enum oEnum(&m_oNodes); !oEnum.AtEnd(); oEnum.Next()) {
      auto const pNode = oEnum.Get();
      nMinLen += pNode->ComputeMinLength();
      if (nMinLen >= Infinity)
        return Infinity;
    }
    return nMinLen;
  }

  void Compile(Compiler*, int) final;
  Node* Simplify(IEnvironment*, LocalHeap*) final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeAnd);
};

/// <remark>
/// Parse tree node for dot(.)
/// </remark>
class NodeAny final : public Node, public WithDirection {
  CASTABLE_FINAL(NodeAny, Node);

 public:
  explicit NodeAny(Direction const direction) : WithDirection(direction) {}

  Op GetOp() const { return IsBackward() ? Op_Any_B : Op_Any_F; }

  // Node
  void Compile(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeAny);
};

/// <remark>
/// Parse tree node for atomic group <c>(?>...)</c>
/// </remark>
class NodeAtom final : public NodeSubNodeBase {
  CASTABLE(NodeAtom, NodeSubNodeBase);

 public:
  explicit NodeAtom(Node* node) : NodeSubNodeBase(node) {}

  // Node
  void Compile(Compiler*, int) final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeAtom);
};

/// <remark>
/// Parse tree node capture. Regex syntax is <c>(...)</c>.
/// </remark>
class NodeCapture final : public NodeSubNodeBase, public WithDirection {
  CASTABLE(NodeCapture, NodeSubNodeBase);

 public:
  /// <summary>
  /// Construct NodeCapture object.
  /// </summary>
  /// <param name="direction">A direction of capturing</param>
  /// <param name="node">A node will be captured</param>
  /// <param name="nth">Capture index number</param>
  NodeCapture(Direction direction, Node* node, int nth)
      : NodeSubNodeBase(node), WithDirection(direction), nth_(nth) {
    DCHECK_GE(nth_, 1);
  }

  // [G]
  /// <summary>
  /// Opcode of this node.
  /// </summary>
  /// <returns>
  /// Op_Capture_B for backward capturing, Op_Capture_F for
  /// forward capturing.
  /// </returns>
  Op GetOp() const { return IsBackward() ? Op_Capture_B : Op_Capture_F; }

  // Node
  void Compile(Compiler*, int) final;
  // Alwasy true for NodeCapture.
  bool NeedStack() const final { return true; }

#if DEBUG_REGEX
  void Print() const final {
    printf("(%s[%d] ", IsBackward() ? "CaptureB" : "CaptureF", nth_);
    GetNode()->Print();
    printf(")");
  }
#endif

 private:
  // An index number of this capture.
  int nth_;

  DISALLOW_COPY_AND_ASSIGN(NodeCapture);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCaptureEq -- \<n> \k<name>
//
class NodeCaptureEq final : public NodeCaptureBase, public WithCase {
  CASTABLE_FINAL(NodeCaptureEq, NodeCaptureBase);

 public:
  NodeCaptureEq(Direction const direction,
                int const nth,
                Case const case_sensitivity)
      : NodeCaptureBase(direction, nth), WithCase(case_sensitivity) {}

  Op GetOp() const {
    return IsBackward()
               ? IsIgnoreCase() ? Op_CaptureEq_Ci_B : Op_CaptureEq_Cs_B
               : IsIgnoreCase() ? Op_CaptureEq_Ci_F : Op_CaptureEq_Cs_F;
  }

  // Node
  void Compile(Compiler*, int) final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeCaptureEq);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCaptureIfNot
//
class NodeCaptureIfNot final : public NodeCaptureBase {
  CASTABLE_FINAL(NodeCaptureIfNot, NodeCaptureBase);

 public:
  NodeCaptureIfNot(Direction const direction, int const nth)
      : NodeCaptureBase(direction, nth) {}

  // Node
  void Compile(Compiler*, int) final { NOTREACHED(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeCaptureIfNot);
};

/// <remark>
/// Parse tree node for character comparison.
/// </remark>
class NodeChar final : public NodeCsBase {
  CASTABLE_FINAL(NodeChar, NodeCsBase);

 public:
  /// <summary>
  /// Construct NodeChar.
  /// </summary>
  NodeChar(Direction const direction,
           char16 char_code,
           Case const case_sensitivity = CaseSensitive,
           bool notEqual = false)
      : NodeCsBase(direction, case_sensitivity, notEqual), char_(char_code) {}

  char16 GetChar() const { return char_; }

  Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
        Op_CharEq_Cs_F, Op_CharEq_Cs_B,

        Op_CharNe_Cs_F, Op_CharNe_Cs_B,

        Op_CharEq_Ci_F, Op_CharEq_Ci_B,

        Op_CharNe_Ci_F, Op_CharNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward())
      k |= 1;
    if (fNot)
      k |= 2;
    if (IsIgnoreCase())
      k |= 4;
    return k_rgeOp[k];
  }

  // Node
  void Compile(Compiler*, int) final;
  void CompileNot(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }
  bool IsCharSetMember(IEnvironment*, char16) const final;
  Node* Simplify(IEnvironment*, LocalHeap*) final;

#if DEBUG_REGEX
  void Print() const final {
    printf("(Char%s%s%s '%c')", IsNot() ? "Ne" : "Eq",
           IsIgnoreCase() ? "Ci" : "Cs", IsBackward() ? "B" : "F", GetChar());
  }
#endif

 private:
  char16 char_;

  DISALLOW_COPY_AND_ASSIGN(NodeChar);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCharClass
//
class NodeCharClass final : public NodeSubNodesBase, public WithDirection {
  CASTABLE_FINAL(NodeCharClass, NodeSubNodesBase);

 public:
  NodeCharClass(Direction const direction, bool const notEqual)
      : WithDirection(direction), not_(notEqual) {}

  bool IsCharSetMember(IEnvironment* pIEnv, char16 wch) const {
    for (Nodes::Enum oEnum(&m_oNodes); !oEnum.AtEnd(); oEnum.Next()) {
      if (oEnum.Get()->IsCharSetMember(pIEnv, wch))
        return true;
    }
    return false;
  }

  bool IsNot() const { return not_; }

  // Node
  void Compile(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }
  bool NeedStack() const final { return true; }
  Node* Simplify(IEnvironment*, LocalHeap*) final;

#if DEBUG_REGEX
  void Print() const final {
    printf("(CharClass%s", IsNot() ? "Not" : "");
    for (Nodes::Enum oEnum(&m_oNodes); !oEnum.AtEnd(); oEnum.Next()) {
      printf(" ");
      oEnum.Get()->Print();
    }
    printf(")");
  }
#endif

 private:
  bool const not_;

  DISALLOW_COPY_AND_ASSIGN(NodeCharClass);
};

//////////////////////////////////////////////////////////////////////
//
// NodeCharSet
//
class NodeCharSet : public NodeEqBase {
  CASTABLE_FINAL(NodeCharSet, NodeEqBase);

 public:
  NodeCharSet(Direction const direction,
              char16* const pwch,
              int const cwch,
              bool const not)
      : NodeEqBase(direction, not), m_cwch(cwch), m_pwch(pwch) {}

  int GetLength() const { return m_cwch; }

  Op GetOp() const {
    return IsBackward() ? IsNot() ? Op_CharSetNe_B : Op_CharSetEq_B
                        : IsNot() ? Op_CharSetNe_F : Op_CharSetEq_F;
  }

  const char16* GetString() const { return m_pwch; }

  // Node
  void Compile(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }

  bool IsCharSetMember(IEnvironment*, char16 wch) const final {
    const char16* pwchEnd = m_pwch + m_cwch;
    for (const char16* pwch = m_pwch; pwch < pwchEnd; pwch++) {
      if (*pwch == wch)
        return !IsNot();
    }
    return IsNot();
  }

#if DEBUG_REGEX
  void Print() const final {
    printf("(%s '%ls')", IsNot() ? "CharSetNe" : "CharSetEq", m_pwch);
  }
#endif

 private:
  int m_cwch;
  char16* m_pwch;

  DISALLOW_COPY_AND_ASSIGN(NodeCharSet);
};

//////////////////////////////////////////////////////////////////////
//
// NodeIf
//
class NodeIf final : public Node {
  CASTABLE_FINAL(NodeIf, Node);

 public:
  NodeIf(Node* pCond, Node* pThen, Node* pElse)
      : m_pCond(pCond), m_pThen(pThen), m_pElse(pElse) {}

  // Node
  void Compile(Compiler*, int) final;
  int ComputeMinLength() const final;
  bool NeedStack() const final { return true; }

  Node* Reverse() final {
    m_pCond = m_pCond->Reverse();
    m_pThen = m_pThen->Reverse();
    m_pElse = m_pElse->Reverse();
    return this;
  }

  Node* Simplify(IEnvironment* pIEnv, LocalHeap* pHeap) final {
    m_pCond = m_pCond->Simplify(pIEnv, pHeap);
    m_pThen = m_pThen->Simplify(pIEnv, pHeap);
    m_pElse = m_pElse->Simplify(pIEnv, pHeap);
    return this;
  }

#if DEBUG_REGEX
  void Print() const final {
    printf("(if ");
    m_pCond->Print();
    printf(" ");
    m_pThen->Print();
    printf(" ");
    m_pElse->Print();
    printf(")");
  }
#endif

 private:
  Node* m_pCond;
  Node* m_pElse;
  Node* m_pThen;

  DISALLOW_COPY_AND_ASSIGN(NodeIf);
};

//////////////////////////////////////////////////////////////////////
//
// NodeLookaround
//
class NodeLookaround final : public NodeSubNodeBase {
  CASTABLE_FINAL(NodeLookaround, NodeSubNodeBase);

 public:
  NodeLookaround(Node* pNode, bool fPositive)
      : NodeSubNodeBase(pNode), m_fPositive(fPositive) {}

  bool IsPositive() const { return m_fPositive; }

  // Node
  void Compile(Compiler*, int) final;
  int ComputeMinLength() const final { return 0; }
  bool NeedStack() const final { return true; }

 private:
  bool const m_fPositive;

  DISALLOW_COPY_AND_ASSIGN(NodeLookaround);
};

//////////////////////////////////////////////////////////////////////
//
// NodeMinMax
// Base class for NodeMax and NodeMin.
//
class NodeMinMax : public NodeSubNodeBase, public WithDirection {
  CASTABLE(NodeMinMax, NodeSubNodeBase);

 public:
  int GetMax() const { return m_iMax; }
  int GetMin() const { return m_iMin; }

  // Node
  int ComputeMinLength() const override {
    return GetNode()->ComputeMinLength() * m_iMin;
  }
  bool NeedStack() const override { return true; }

#if DEBUG_REGEX
  void Print() const override {
    printf("(%s min=%d max=%d ", GetKind(), GetMin(), GetMax());
    GetNode()->Print();
    printf(")");
  }
#endif

 protected:
  NodeMinMax(Direction const direction, Node* const node, MinMax minmax)
      : NodeSubNodeBase(node),
        WithDirection(direction),
        m_iMax(minmax.m_iMax),
        m_iMin(minmax.m_iMin) {}

 private:
  int const m_iMax;
  int const m_iMin;

  DISALLOW_COPY_AND_ASSIGN(NodeMinMax);
};

//////////////////////////////////////////////////////////////////////
//
// NodeMax
//
class NodeMax final : public NodeMinMax {
  CASTABLE_FINAL(NodeMax, NodeMinMax);

 public:
  NodeMax(Direction direction, Node* node, MinMax minmax)
      : NodeMinMax(direction, node, minmax) {}

  // Node
  void Compile(Compiler*, int) final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeMax);
};

//////////////////////////////////////////////////////////////////////
//
// NodeMin
//
class NodeMin final : public NodeMinMax {
  CASTABLE_FINAL(NodeMin, NodeMinMax);

 public:
  NodeMin(Direction direction, Node* node, MinMax minmax)
      : NodeMinMax(direction, node, minmax) {}

  // Node
  void Compile(Compiler*, int) final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeMin);
};

//////////////////////////////////////////////////////////////////////
//
// NodeOneWidth
//
class NodeOneWidth final : public NodeOpBase {
  CASTABLE_FINAL(NodeOneWidth, NodeOpBase);

 public:
  explicit NodeOneWidth(Op opcode) : NodeOpBase(opcode) {}

  Op GetNotOp() const { return static_cast<Op>(GetOp() + 1); }

  // Node
  void CompileNot(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }
  bool IsCharSetMember(IEnvironment*, char16 wch) const final;
  Node* Not() final;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeOneWidth);
};

//////////////////////////////////////////////////////////////////////
//
// NodeOr
//
class NodeOr final : public NodeSubNodesBase {
  CASTABLE_FINAL(NodeOr, NodeSubNodesBase);

 public:
  explicit NodeOr(Node* pNode1) { m_oNodes.Append(pNode1); }
  NodeOr(Node* pNode1, Node* pNode2) {
    m_oNodes.Append(pNode1);
    m_oNodes.Append(pNode2);
  }
  NodeOr() {}

  // ComputeMinLength - minimum value of all subnodes.
  int ComputeMinLength() const final {
    auto nMinLen = static_cast<int>(Infinity);
    for (Nodes::Enum oEnum(&m_oNodes); !oEnum.AtEnd(); oEnum.Next()) {
      auto const pNode = oEnum.Get();
      nMinLen = std::min(nMinLen, pNode->ComputeMinLength());
    }
    return nMinLen;
  }

  void Compile(Compiler*, int) final;
  Nodes* GetNodes() { return &m_oNodes; }
  bool NeedStack() const final { return true; }

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeOr);
};

//////////////////////////////////////////////////////////////////////
//
// NodeRange
// Represent simple character class, e.g. [A-Z].
//
class NodeRange final : public NodeCsBase {
  CASTABLE_FINAL(NodeRange, NodeCsBase);

 public:
  NodeRange(Direction direction,
            char16 wchMin,
            char16 wchMax,
            Case case_sensitivity,
            bool notEqual = false)
      : NodeCsBase(direction, case_sensitivity, notEqual),
        m_wchMin(wchMin),
        m_wchMax(wchMax) {}

  void Compile(Compiler*, int) final;
  void CompileNot(Compiler*, int) final;
  int ComputeMinLength() const final { return 1; }
  char16 GetMaxChar() const { return m_wchMax; }
  char16 GetMinChar() const { return m_wchMin; }

  Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
        Op_RangeEq_Cs_F, Op_RangeEq_Cs_B,

        Op_RangeNe_Cs_F, Op_RangeNe_Cs_B,

        Op_RangeEq_Ci_F, Op_RangeEq_Ci_B,

        Op_RangeNe_Ci_F, Op_RangeNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward())
      k |= 1;
    if (fNot)
      k |= 2;
    if (IsIgnoreCase())
      k |= 4;
    return k_rgeOp[k];
  }

  bool IsCharSetMember(IEnvironment*, char16) const final;
  Node* Simplify(IEnvironment*, LocalHeap*) final;

#if DEBUG_REGEX
  void Print() const override {
    printf("(Range%s%s %lc-%lc)", IsNot() ? "Not" : "",
           IsIgnoreCase() ? "Ci" : "Cs", GetMinChar(), GetMaxChar());
  }
#endif

 private:
  char16 const m_wchMin;
  char16 const m_wchMax;

  DISALLOW_COPY_AND_ASSIGN(NodeRange);
};

/// <remark>
/// Parse tree node for string comparison.
/// </remark>
class NodeString final : public NodeCsBase {
  CASTABLE_FINAL(NodeString, NodeCsBase);

 public:
  NodeString(Direction direction,
             const char16* pwch,
             int cwch,
             Case case_sensitivity = CaseSensitive,
             bool notEqual = false);

  int ComputeMinLength() const final { return m_cwch; }
  void Compile(Compiler*, int) final;
  int GetLength() const { return m_cwch; }

  Op GetOp(bool fNot) const {
    static const Op k_rgeOp[] = {
        Op_StringEq_Cs_F, Op_StringEq_Cs_B,

        Op_StringNe_Cs_F, Op_StringNe_Cs_B,

        Op_StringEq_Ci_F, Op_StringEq_Ci_B,

        Op_StringNe_Ci_F, Op_StringNe_Ci_B,
    };

    auto k = 0;
    if (IsBackward())
      k |= 1;
    if (fNot)
      k |= 2;
    if (IsIgnoreCase())
      k |= 4;
    return k_rgeOp[k];
  }

  const char16* GetStart() const { return m_pwch; }

// [P]
#if DEBUG_REGEX
  void Print() const final {
    printf("(String%s%s%s '%ls')", IsNot() ? "Ne" : "Eq",
           IsIgnoreCase() ? "Ci" : "Cs", IsBackward() ? "B" : "F", m_pwch);
  }
#endif

 private:
  int const m_cwch;
  const char16* const m_pwch;

  DISALLOW_COPY_AND_ASSIGN(NodeString);
};

/// <summary>
/// Parse tree node for void. This is used for empty capture.
/// </summary>
class NodeVoid final : public Node {
  CASTABLE_FINAL(NodeVoid, Node);

 public:
  void Compile(Compiler*, int) override {}
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
class NodeZeroWidth final : public NodeOpBase {
  CASTABLE_FINAL(NodeZeroWidth, NodeOpBase);

 public:
  explicit NodeZeroWidth(Op opcode) : NodeOpBase(opcode) {}
};

//////////////////////////////////////////////////////////////////////
//
// CaptureDef
//
struct CaptureDef : LocalObject, DoubleLinkedItem_<CaptureDef> {
  int m_iNth;
  char16* m_pwszName;

  CaptureDef(char16* pwszName, int iNth) : m_iNth(iNth), m_pwszName(pwszName) {}
};

typedef DoubleLinkedList_<CaptureDef> CaptureDefs;

//////////////////////////////////////////////////////////////////////
//
// Tree
//
class Tree final : public LocalObject {
 public:
  explicit Tree(int rgfOption)
      : m_cCaptures(0),
        m_iErrorCode(0),
        m_lErrorPosn(0),
        m_pNode(nullptr),
        m_rgfOption(rgfOption),
        m_prgpwszCaptureName(nullptr) {}

  // TODO(eval1749) These member variables should be private.
  int m_cCaptures;
  int m_iErrorCode;
  int m_lErrorPosn;
  CaptureDefs m_oCaptures;
  Node* m_pNode;
  int m_rgfOption;
  char16* m_prgpwszCaptureName;
};

}  // namespace RegexPrivate
}  // namespace Regex

#endif  // REGEX_REGEX_NODE_H_
