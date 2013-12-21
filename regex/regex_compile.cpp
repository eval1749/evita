#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Regex - Compiler
// regex/regex_compiler.cpp
//
// Copyright (C) 2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_compile.cpp#9 $
//
#include "./IRegex.h"
#include "./regex_bytecode.h"
#include "./regex_node.h"
#include "./regex_scanner.h"
#include <algorithm>

namespace Regex
{

namespace RegexPrivate
{

typedef Regex::ICompileContext ICompileContext;

static Op
computeRepeatCharOp(const NodeChar* pChar)
{
    static const Op k_rgeOp[] =
    {
        Op_RepeatCharEq_Cs_F,
        Op_RepeatCharEq_Cs_B,

        Op_RepeatCharNe_Cs_F,
        Op_RepeatCharNe_Cs_B,

        Op_RepeatCharEq_Ci_F,
        Op_RepeatCharEq_Ci_B,

        Op_RepeatCharNe_Ci_F,
        Op_RepeatCharNe_Ci_B,
    }; // k_rgeOp

    int k = 0;
    if (pChar->IsBackward()) k |= 1;
    if (pChar->IsNot()) k |= 2;
    if (pChar->IsIgnoreCase()) k |= 4;
    return k_rgeOp[k];
} // computeRepeatCharOp

static Op
computeRepeatCharSetOp(const NodeCharSet* pCharSet)
{
    static const Op k_rgeOp[] =
    {
        Op_RepeatCharSetEq_F,
        Op_RepeatCharSetEq_B,

        Op_RepeatCharSetNe_F,
        Op_RepeatCharSetNe_B,
    }; // k_rgeOp

    int k = 0;
    if (pCharSet->IsBackward()) k |= 1;
    if (pCharSet->IsNot()) k |= 2;
    return k_rgeOp[k];
} // computeRepeatCharSetOp

static Op
computeRepeatOneWidthOp(Op eOp)
{
    return static_cast<Op>(
        Op_RepeatAsciiDigitCharEq_B + eOp - Op_AsciiDigitCharEq_B );
} // computeRepeatOneWidthOp

static Op
computeRepeatRangeOp(const NodeRange* pRange)
{
    static const Op k_rgeOp[] =
    {
        Op_RepeatRangeEq_Cs_F,
        Op_RepeatRangeEq_Cs_B,

        Op_RepeatRangeNe_Cs_F,
        Op_RepeatRangeNe_Cs_B,

        Op_RepeatRangeEq_Ci_F,
        Op_RepeatRangeEq_Ci_B,

        Op_RepeatRangeNe_Ci_F,
        Op_RepeatRangeNe_Ci_B,
    }; // k_rgeOp

    int k = 0;
    if (pRange->IsBackward()) k |= 1;
    if (pRange->IsNot()) k |= 2;
    if (pRange->IsIgnoreCase()) k |= 4;
    return k_rgeOp[k];
} // computeRepeatRangeOp

static Op
computeStringOp(bool fBackward, bool fIgnoreCase, bool fNot)
{
    static const Op k_rgeOp[] =
    {
        Op_StringEq_Cs_F,
        Op_StringEq_Cs_B,

        Op_StringNe_Cs_F,
        Op_StringNe_Cs_B,

        Op_StringEq_Ci_F,
        Op_StringEq_Ci_B,

        Op_StringNe_Ci_F,
        Op_StringNe_Ci_B,
    }; // k_rgeOp

    int k = 0;
    if (fBackward) k |= 1;
    if (fNot) k |= 2;
    if (fIgnoreCase) k |= 4;
    return k_rgeOp[k];
} // computeStringOp

static void fillChar(
    char16* pwchStart,
    char16  wch,
    int     cwch )
{
    char16* pwchEnd = pwchStart + cwch;
    for (char16* pwch = pwchStart; pwch < pwchEnd; pwch++)
    {
        *pwch = wch;
    }
} // fillChar

//////////////////////////////////////////////////////////////////////
//
// LoopInfo
//  Represents result of foldNestedLoop
//
struct LoopInfo
{
    int     m_iMax;
    int     m_iMin;
    Node*   node_;

    LoopInfo(
        int     iMin,
        int     iMax,
        Node*   pNode ) :
        m_iMax(iMax),
        m_iMin(iMin),
        node_(pNode) {}
}; // LoopInfo

// maxMul
static int maxMul(int max1, int max2)
{
    if (0 == max1) return 0;
    if (0 == max2) return 0;
    if (max1 == Infinity) return max1;
    if (max2 == Infinity) return max1;
    return max1 * max2;
} // maxMul

//////////////////////////////////////////////////////////////////////
//
// Fold nested loop:
//  (max min-1 max-1 (max min-2 max-2 subNode))
//      where min-2 /= max-2
//  ==>
//  (max (* min-1 min-2) (* max-1 max-2) subNode)
//
//  Example:
//    (a{1,4})* => a+
//    (a{4})*   => (a{4})*
//
static LoopInfo foldNestedLoop(
    Node*   pNode,
    int     iMin,
    int     iMax,
    Node*   pSubNode )
{
    for (;;)
    {
        if (pNode->GetKind() != pSubNode->GetKind())
        {
            return LoopInfo(iMin, iMax, pSubNode);
        }

        int iSubMin;
        int iSubMax;

        Node* pSubSubNode;
        if (pSubNode->Is<NodeMax>())
        {
            iSubMin     = pSubNode->StaticCast<NodeMax>()->GetMin();
            iSubMax     = pSubNode->StaticCast<NodeMax>()->GetMax();
            pSubSubNode = pSubNode->StaticCast<NodeMax>()->GetNode();
        }
        else
        {
            iSubMin     = pSubNode->StaticCast<NodeMin>()->GetMin();
            iSubMax     = pSubNode->StaticCast<NodeMin>()->GetMax();
            pSubSubNode = pSubNode->StaticCast<NodeMin>()->GetNode();
        }

        if (iSubMin == iSubMax)
        {
            return LoopInfo(iMin, iMax, pSubNode);
        }

        iMin *= iSubMin;
        iMax = maxMul(iMax, iSubMax);
        pSubNode = pSubSubNode;
    } // for
} // foldNestedLoop

class CompilerObject :
    public DoubleLinkedItem_<CompilerObject>,
    public LocalObject
{
    private: int m_nPc;

    protected: CompilerObject() :
        m_nPc(0) {}

    // [G]
    public: int            GetPc() { return m_nPc; }
    public: virtual size_t GetSize() const = 0;

    // [S]
    public: void           SetPc(int nPc) { m_nPc = nPc; }
    public: virtual void   Serialize(void*) const = 0;
}; // CompilerObjet

typedef DoubleLinkedList_<CompilerObject> CompilerObjects;

class CompilerString :
    public CompilerObject
{
    private: int        m_cwch;
    private: char16*    m_prgwch;

    // ctor
    public: CompilerString(LocalHeap* pHeap, const char16* pwch, int cwch) :
        m_cwch(cwch)
    {
        m_prgwch = reinterpret_cast<char16*>(
            pHeap->Alloc(sizeof(char16) * m_cwch) );

        ::CopyMemory(m_prgwch, pwch, sizeof(char16) * m_cwch);
    } // CompilerString

    public: CompilerString(LocalHeap* pHeap, int cwch) :
        m_cwch(cwch)
    {
        m_prgwch = reinterpret_cast<char16*>(
            pHeap->Alloc(sizeof(char16) * m_cwch) );
    } // CompilerString

    // [G]
    public: char16* Get() const { return m_prgwch; }

    public: virtual size_t GetSize() const override
    {
        size_t cb = sizeof(StringOperand);
        cb  = sizeof(char16) * (m_cwch + 1);
        cb  = (cb + sizeof(int) * 8 - 1);
        cb /= (sizeof(int) * 8);
        cb *= sizeof(int) * 8;
        return cb;
    } // GetSize

    public: virtual void Serialize(void* pv) const override
    {
        StringOperand* p = reinterpret_cast<StringOperand*>(pv);
        p->m_cwch = m_cwch;
        char16* pwch = reinterpret_cast<char16*>(p + 1);
        CopyMemory(pwch, m_prgwch, sizeof(char16) * m_cwch);
        pwch[m_cwch] = 0;
    } // Serialize
}; // CompilerString

class ScannerCompiler : public LocalObject
{
    protected: ICompileContext* m_pIContext;

    protected: ScannerCompiler(ICompileContext* pIContext) :
        m_pIContext(pIContext) {}

    // [C]
    public: virtual int ComputeMinLength() const = 0;

    // [G]
    public: virtual size_t GetSize() const = 0;

    // [S]
    public: virtual void Serialize(void*) const = 0;
}; // ScannerCompiler

class CharScannerCompiler :
    public ScannerCompiler,
    public CharScanner
{
    public: CharScannerCompiler(
        ICompileContext*    pIContext,
        NodeChar*           pChar ) :
            CharScanner(
                pChar->GetChar(),
                pChar->IsBackward(),
                pChar->IsIgnoreCase() ),
            ScannerCompiler(pIContext) {}

    // [C]
    public: virtual int ComputeMinLength() const
        { return 1; }

    // [G]
    private: virtual size_t GetSize() const
        { return sizeof(CharScanner); }

    // [S]
    private: virtual void Serialize(void* pv) const
    {
        CharScanner* p = reinterpret_cast<CharScanner*>(pv);
        *p = *this;
    } // Serialize
}; // CharScannerCompiler

class FullScannerCompiler :
    public ScannerCompiler,
    public FullScanner
{
    public: FullScannerCompiler(ICompileContext* pIContext, bool fBackward) :
        ScannerCompiler(pIContext),
        FullScanner(fBackward) {}

    // [C]
    public: virtual int ComputeMinLength() const
        { return 1; }

    // [G]
    private: virtual size_t GetSize() const
        { return sizeof(FullScanner); }

    // [S]
    private: virtual void Serialize(void* pv) const
    {
        FullScanner* p = reinterpret_cast<FullScanner*>(pv);
        *p = *this;
    } // Serialize
}; // FullScannerCompiler

/// <remark>
///   String scanner compiler
/// </remark>
class StringScannerCompiler :
    public ScannerCompiler,
    protected StringScanner
{
    private: NodeString*    m_pString;

    public: StringScannerCompiler(
        ICompileContext*    pIContext,
        NodeString*         pString ) :
            ScannerCompiler(pIContext),
            StringScanner(
                pString->IsBackward(),
                pString->IsIgnoreCase() ),
            m_pString(pString)
    {
        m_cwch = pString->GetLength();

        m_nMaxChar = 0x0000u;
        m_nMinChar = 0xffffu;
        m_iShift   = pString->GetLength() + 1;

        if (m_pString->IsBackward())
        {
            m_iShift = -m_iShift;
        }

        foreach (EnumChar, oEnum, EnumChar::Arg(pString->GetStart(), m_cwch))
        {
            char16 wch = oEnum.Get();
            if (m_pString->IsIgnoreCase())
            {
                wch = m_pIContext->CharUpcase(wch);
            }
            m_nMaxChar = std::max(m_nMaxChar, static_cast<int>(wch));
            m_nMinChar = std::min(m_nMinChar, static_cast<int>(wch));
        } // for pwch

        ASSERT(m_nMinChar <= m_nMaxChar);
    } // StringScannerCompiler

    // [C]
    public: virtual int ComputeMinLength() const
        { return m_cwch; }

    // [G]
    private: virtual size_t GetSize() const
    {
        size_t cb = sizeof(StringScanner);
        cb += sizeof(int) * (m_nMaxChar - m_nMinChar + 1);
        cb += sizeof(char16) * m_cwch;
        return cb;
    } // GetSize

    // [S]
    private: virtual void Serialize(void* pv) const
    {
        ASSERT(m_nMinChar <= m_nMaxChar);

        StringScanner* p = reinterpret_cast<StringScanner*>(pv);
        *p = *this;

        int* prgi = reinterpret_cast<int*>(p + 1);

        char16* pwch = reinterpret_cast<char16*>(
            prgi + (m_nMaxChar - m_nMinChar + 1) );

        ::CopyMemory(
            pwch,
            m_pString->GetStart(),
            sizeof(char16) * m_cwch );

        int m = m_pString->GetLength();

        for (int i = 0; i <= (m_nMaxChar - m_nMinChar); i++)
        {
            prgi[i] = m_iShift;
        } // for i

        if (m_pString->IsBackward())
        {
            for (int i = 0; i < m; i++)
            {
                char16 wch = m_pString->GetStart()[m - i - 1];

                if (m_pString->IsIgnoreCase())
                {
                    wch = m_pIContext->CharUpcase(wch);
                }

                prgi[wch - m_nMinChar] = i - m;
            } // for i
        }
        else
        {
            for (int i = 0; i < m; i++)
            {
                char16 wch = m_pString->GetStart()[i];

                if (m_pString->IsIgnoreCase())
                {
                    wch = m_pIContext->CharUpcase(wch);
                }

                prgi[wch - m_nMinChar] = m - i;
            } // for i
        }
    } // Serialize
}; // StringScannerCompiler

class ZeroWidthScannerCompiler :
    public ScannerCompiler,
    public ZeroWidthScanner
{
    public: ZeroWidthScannerCompiler(
        ICompileContext*    pIContext,
        Op                  eOp ) :
            ScannerCompiler(pIContext),
            ZeroWidthScanner(eOp) {}
    // [C]
    public: virtual int ComputeMinLength() const
        { return 0; }

    // [G]
    private: virtual size_t GetSize() const
        { return sizeof(ZeroWidthScanner); }

    // [S]
    private: virtual void Serialize(void* pv) const
    {
        ZeroWidthScanner* p = reinterpret_cast<ZeroWidthScanner*>(pv);
        *p = *this;
    } // Serialize
}; // ZeroWidthScannerCompiler

//////////////////////////////////////////////////////////////////////
//
// Compiler
//
class Compiler
{
    private: int                m_nOpLastPc;
    private: int                m_nLoopDepth;
    private: int                m_nMinRest;
    private: LocalHeap*         m_pHeap;
    private: ICompileContext*   m_pIContext;
    private: ScannerCompiler*   m_pScannerCompiler;
    private: Sink<int>          m_oCodeSink;
    private: CompilerObjects    m_oOperands;
    public:  int                m_rgfOption;

    public: Compiler(
        ICompileContext*    pIContext,
        LocalHeap*          pHeap ):
            m_nOpLastPc(0),
            m_nLoopDepth(0),
            m_nMinRest(0),
            m_oCodeSink(pHeap),
            m_pIContext(pIContext),
            m_pHeap(pHeap),
            m_pScannerCompiler(NULL),
            m_rgfOption(0) {}

    // [C]

    // Note: If you add new scan method, you must implement scanner in
    // Engine::Execute in "regex_parse.cpp".
    private: Node* compileScanner(Node* pNode, NodeAnd* pParent)
    {
        if (NodeAnd* pAnd = pNode->DynamicCast<NodeAnd>())
        {
            compileScanner(pAnd->GetFirst(), pAnd);
            return pNode;
        }

        if (NodeChar* pChar = pNode->DynamicCast<NodeChar>())
        {
            if (pChar->IsNot())
            {
                // FIXME 2008-07-05 yosi@msn.com We should have char-not
                // scanner.
                return pNode;
            }

            m_pScannerCompiler = new(m_pHeap) CharScannerCompiler(
                m_pIContext,
                pChar );

            if (NULL == pParent)
            {
                return NULL;
            }

            pParent->Delete(pNode);
            return pParent;
        } // if char

        if (NodeString* pString = pNode->DynamicCast<NodeString>())
        {
            m_pScannerCompiler = new(m_pHeap) StringScannerCompiler(
                m_pIContext,
                pString );

            if (NULL == pParent)
            {
                return NULL;
            }

            pParent->Delete(pNode);
            return pParent;
        } // if string

        if (NodeZeroWidth* pZeroWidth = pNode->DynamicCast<NodeZeroWidth>())
        {
            switch (pZeroWidth->GetOp())
            {
            case Op_AfterNewline:
            case Op_BeforeNewline:
            case Op_EndOfLine:
            case Op_EndOfString:
            case Op_Posn:
            case Op_StartOfString:
                m_pScannerCompiler = new(m_pHeap) ZeroWidthScannerCompiler(
                    m_pIContext,
                    pZeroWidth->GetOp() );

                if (NULL == pParent)
                {
                    return NULL;
                }

                pParent->Delete(pNode);
                return pParent;
            } // switch op

            return pNode;
        } // if zero width

        return pNode;
    } // compileScanner

    public: RegexObj* Compile(Tree* pTree)
    {
        m_rgfOption = pTree->m_rgfOption;

        Node* pRootNode = compileScanner(pTree->m_pNode, NULL);
        if (NULL == pRootNode)
        {
            pRootNode = new(m_pHeap) NodeVoid;
        }

        if (NULL == m_pScannerCompiler)
        {
            m_pScannerCompiler = new(m_pHeap) FullScannerCompiler(
                m_pIContext,
                0 != (m_rgfOption & Option_Backward) );
        }

        pRootNode->Compile(this, 0);
        Emit(Op_Success);
        Emit(Op_End);

        size_t cbOperands = 0;
        foreach (CompilerObjects::Enum, oEnum, &m_oOperands)
        {
            cbOperands += oEnum.Get()->GetSize();
        } // for each object

        size_t cbScanner = m_pScannerCompiler->GetSize();

        size_t cbRegex = sizeof(RegexObj);

        size_t ofsCode = cbRegex;
        cbRegex += m_oCodeSink.GetSize();
        cbRegex += cbOperands;

        size_t ofsScanner = cbRegex;
        cbRegex += cbScanner;

        void* pv = m_pIContext->AllocRegex(cbRegex,  pTree->m_cCaptures);

        if (NULL == pv)
        {
            m_pIContext->SetError(0, Regex::Error_NotEnoughMemory);
            return NULL;
        }

        foreach (CaptureDefs::Enum, oEnum, &pTree->m_oCaptures)
        {
            CaptureDef* pDef = oEnum.Get();
            m_pIContext->SetCapture(pDef->m_iNth, pDef->m_pwszName);
        } // for each capture

        // Note: We don't count scanner's match length into regex minlen.
        int nMinLen = pRootNode->ComputeMinLength();

        serializeCode(
            reinterpret_cast<uint8*>(pv) + ofsCode );

        m_pScannerCompiler->Serialize(
            reinterpret_cast<uint8*>(pv) + ofsScanner );

        RegexObj* pRegex = new(pv) RegexObj(
            m_rgfOption,
            pTree->m_cCaptures,
            nMinLen,
            static_cast<int>(ofsCode),
            static_cast<int>(ofsScanner) );

        #if _DEBUG
            pRegex->Describe();
            printf("\n\n");
        #endif

        return pRegex;
    } // Compile

    public: bool CompileMaxFixed(int, Node*, int);
    public: bool CompileMaxSimple(const LoopInfo*, int);
    public: void CompileRange(const NodeRange*, bool);

    public: CompilerString* CreateString(const char16* pwch, int cwch)
    {
        return new(m_pHeap) CompilerString(m_pHeap, pwch, cwch);
    } // CreateString

    // [E]
    public: void Emit(Op eOp)
        { m_oCodeSink.Add(eOp); }

    public: void Emit(Op eOp, int a)
        { m_oCodeSink.Add(eOp); m_oCodeSink.Add(a); }

    public: void Emit(Op eOp, CompilerObject* pObject)
    {
        m_oCodeSink.Add(eOp);
        pObject->SetPc(GetPc());
        m_oOperands.Append(pObject);
        m_oCodeSink.Add(0);
    } // Emit

    public: void Emit(Op eOp, int a, int b)
    {
        m_oCodeSink.Add(eOp);
        m_oCodeSink.Add(a);
        m_oCodeSink.Add(b);
    } // Emit

    public: void Emit(Op eOp, int a, int b, int c)
    {
        m_oCodeSink.Add(eOp);
        m_oCodeSink.Add(a);
        m_oCodeSink.Add(b);
        m_oCodeSink.Add(c);
    } // Emit

    public: void Emit(Op eOp, int a, CompilerObject* pObject)
    {
        m_oCodeSink.Add(eOp);
        m_oCodeSink.Add(a);
        pObject->SetPc(GetPc());
        m_oOperands.Append(pObject);
        m_oCodeSink.Add(0);
    } // Emit

    public: int EmitRefLabel(Op eOp, int nLinkPc = 0)
    {
        m_oCodeSink.Add(eOp);
        int nRefPc = m_oCodeSink.GetLength();
        m_oCodeSink.Add(nLinkPc);
        return nRefPc;
    } // EmitRefLabel

    public: void EndLoop()
        { m_nLoopDepth -= 1; }

    // [G]
    public: int GetPc() const
        { return m_oCodeSink.GetLength(); }

    public: LocalHeap* GetHeap() const
        { return m_pHeap; }

    // [I]
    public: bool IsInLoop() const
        { return 1 == m_nLoopDepth; }

    // [P]
    public: void PatchLabel(int nRefPc)
    {
        #if _DEBUG
            printf("PatchLabel: @%d <- L%04X\n", nRefPc, GetPc());
        #endif

        m_oCodeSink.Set(nRefPc, GetPc());
    } // PatchLabel

    public: void PatchLabels(int nRefPc)
    {
        while (0 != nRefPc)
        {
            int nNextPc = m_oCodeSink.Get(nRefPc);
            PatchLabel(nRefPc);
            nRefPc = nNextPc;
        } // nRefPc
    } // PatchLabels

    public: void PatchOpLast()
    {
        if (0 != m_nOpLastPc)
        {
            m_oCodeSink.Set(
                m_nOpLastPc,
                Op_Last_B == m_oCodeSink.Get(m_nOpLastPc) ?
                    Op_Rest_B : Op_Rest_F );
        }

        m_nOpLastPc = GetPc();
    } // PatchOpLast

    // [S]
    private: char16* saveString(const char16* pwszSrc)
    {
        int cwch = lstrlen(pwszSrc);
        char16* pwszNew = new char16[cwch + 1];
        if (NULL == pwszNew) return pwszNew;
        const char16* pwszEnd = pwszNew + cwch;
        char16* pwsz;
        for (pwsz = pwszNew; pwsz < pwszEnd; pwsz++)
        {
            *pwsz = *pwszSrc++;
        } // for
        *pwsz = 0;
        return pwszNew;
    } // saveString

    //  Serialize bytecode into memory.
    private: void serializeCode(void* pv) const
    {
        uint8* prgnCode = reinterpret_cast<uint8*>(pv);

        m_oCodeSink.Serialize(prgnCode);

        uint8* pbOperands = prgnCode + m_oCodeSink.GetSize();
        foreach (CompilerObjects::Enum, oEnum, &m_oOperands)
        {
            CompilerObject* pObject = oEnum.Get();

            // Set operand
            reinterpret_cast<int*>(prgnCode)[pObject->GetPc()] =
                static_cast<int>(pbOperands - prgnCode);

            pObject->Serialize(pbOperands);

            pbOperands += pObject->GetSize();
        } // for each object
    } // serializeCode

    public: void StartLoop()
        { m_nLoopDepth += 1; }
}; // Compiler

void NodeAnd::Compile(Compiler* pCompiler, int nMinRest)
{
    foreach (Nodes::Enum, oEnum, &m_oNodes)
    {
        Node* pNode = oEnum.Get();
        nMinRest += pNode->ComputeMinLength();
    } // for each node

    foreach (Nodes::Enum, oEnum, &m_oNodes)
    {
        Node* pNode = oEnum.Get();
        nMinRest -= pNode->ComputeMinLength();
        pNode->Compile(pCompiler, nMinRest);
    } // for each node
} // NodeAnd::Compile

void NodeAny::Compile(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetOp());
} // NodeAny::Compile

void NodeAtom::Compile(Compiler* pCompiler, int)
{
    if (NeedStack())
    {
        pCompiler->Emit(Op_SaveCxp);
        GetNode()->Compile(pCompiler, 0);
        pCompiler->Emit(Op_RestoreCxp);
    }
    else
    {
        GetNode()->Compile(pCompiler, 0);
    }
} // NodeAtom::Compile

void NodeCapture::Compile(Compiler* pCompiler, int nMinRest)
{
    pCompiler->Emit(Op_PushPosn);
    GetNode()->Compile(pCompiler, nMinRest);
    pCompiler->Emit(GetOp(), nth_);
} // NodeCapture::Compile

void NodeCaptureEq::Compile(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetOp(), GetNth());
} // NodeCaptureEq::Compile

void NodeChar::Compile(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetOp(IsNot()), char_);
} // NodeChar::Compile

void NodeChar::CompileNot(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetOp(! IsNot()), char_); 
} // NodeChar::CompileNot

void NodeCharSet::Compile(Compiler* pCompiler, int)
{
    CompilerString* pString = pCompiler->CreateString(m_pwch, m_cwch);
    pCompiler->Emit(GetOp(), pString);
} // NodeCharSet::Compile

//////////////////////////////////////////////////////////////////////
//
// NodeCharClass::Compile
//
//  (char-class cc1 cc2 cc3)
//      OR L_1
//      cc1
//      GO L_end
//     L_1:
//      OR L_2
//      cc2
//      GO L_end
//     L_2:
//      cc3
//     L_end:
//
//  (not-char-class cc1 cc2 cc3)
//      not cc1
//      ANY_B
//      not cc2
//      ANY_B
//      not cc3
//
void NodeCharClass::Compile(Compiler* pCompiler, int nMinRest)
{
    if (NULL == m_oNodes.GetFirst())
    {
        // (or) => nothing (always succeeded)
        return;
    }

    if (IsNot())
    {
        if (NULL == m_oNodes.GetFirst()->GetNext())
        {
            // (or re) => re
            m_oNodes.GetFirst()->CompileNot(pCompiler, nMinRest);
            return;
        }

        foreach (Nodes::Enum, oEnum, &m_oNodes)
        {
            Node* pNode = oEnum.Get();
            pNode->CompileNot(pCompiler, nMinRest);
            if (NULL != pNode->GetNext())
            {
                pCompiler->Emit(IsBackward() ?  Op_Any_F : Op_Any_B);
            }
        } // for each node
    }
    else
    {
        if (NULL == m_oNodes.GetFirst()->GetNext())
        {
            // (or re) => re
            m_oNodes.GetFirst()->Compile(pCompiler, nMinRest);
            return;
        }

        int nEndPc = 0;
        foreach (Nodes::Enum, oEnum, &m_oNodes)
        {
            Node* pNode = oEnum.Get();

            int nPushPc = 0;
            if (NULL != pNode->GetNext())
            {
                // FIXME 2007-08-11 yosi@msn.com NYI emit pop if all
                // altanatives are distinct.
                nPushPc = pCompiler->EmitRefLabel(Op_Or, nPushPc);
            }

            pNode->Compile(pCompiler, nMinRest);

            if (0 != nPushPc)
            {
                nEndPc = pCompiler->EmitRefLabel(Op_Go, nEndPc);
                pCompiler->PatchLabel(nPushPc);
            }
        } // for each Node

        pCompiler->PatchLabels(nEndPc);
    } // if
} // NodeChar::Compile

void NodeIf::Compile(Compiler* pCompiler, int nMinRest)
{
    // if capture-if-not then else
    //      CAPTURE-IF-NO   L001
    //      ... then ...
    //      GO L002
    //    L001:
    //      ... else ...
    //    L002:
    {
        NodeCaptureIfNot* pCaptureIfNot =
            m_pCond->DynamicCast<NodeCaptureIfNot>();

        if (NULL != pCaptureIfNot)
        {
            int iPushPc = pCompiler->GetPc() + 1;

            pCompiler->Emit(Op_CaptureIfNot, 0, pCaptureIfNot->GetNth());

            m_pThen->Compile(pCompiler, nMinRest);

            if (! m_pElse->Is<NodeVoid>())
            {
                int iBranchPc = pCompiler->EmitRefLabel(Op_Go);
                pCompiler->PatchLabel(iPushPc);
                iPushPc = iBranchPc;

                m_pElse->Compile(pCompiler, nMinRest);
            } // if

            pCompiler->PatchLabel(iPushPc);
            return;
        } // if
    }

    // looaround
    //      SAVE-CXP
    //      OR L001
    //      ... lookaround ...
    //      RESTORE-CXP
    //      ... then ...
    //      GO L002
    //    L001:
    //      RESTORE-CXP
    //      ... else ...
    //    L002:
    {
        NodeLookaround* pLookaround =
            m_pCond->DynamicCast<NodeLookaround>();

        if (NULL != pLookaround)
        {
            pCompiler->Emit(Op_SaveCxp);
            int iPushPc = pCompiler->EmitRefLabel(Op_Or);
            m_pCond->Compile(pCompiler, nMinRest);

            pCompiler->Emit(Op_RestoreCxp);
            m_pThen->Compile(pCompiler, nMinRest);

            if (! m_pElse->Is<NodeVoid>())
            {
                int iBranchPc = pCompiler->EmitRefLabel(Op_Go);
                pCompiler->PatchLabel(iPushPc);
                iPushPc = iBranchPc;

                pCompiler->Emit(Op_RestoreCxp);
                m_pElse->Compile(pCompiler, nMinRest);
            } // if

            pCompiler->PatchLabel(iPushPc);
            return;
        } // if
    }

    CAN_NOT_HAPPEN();
} // NodeIf::Compile

void NodeLookaround::Compile(Compiler* pCompiler, int nMinRest)
{
    if (IsPositive())
    {
        pCompiler->Emit(Op_SaveCxp);
        pCompiler->Emit(Op_SavePosn);
        GetNode()->Compile(pCompiler, nMinRest);
        pCompiler->Emit(Op_RestorePosn);
        pCompiler->Emit(Op_RestoreCxp);
    }
    else
    {
        if (GetNode()->Is<NodeVoid>())
        {
            pCompiler->Emit(Op_Fail);
            return;
        }

        pCompiler->Emit(Op_SaveCxp);
        int nPatchPc = pCompiler->EmitRefLabel(Op_Or);
        GetNode()->Compile(pCompiler, nMinRest);
        pCompiler->Emit(Op_RestoreCxp);
        pCompiler->Emit(Op_Fail);
        pCompiler->PatchLabel(nPatchPc);
        pCompiler->Emit(Op_RestoreCxp);
    }
} // NodeLookaround::Compile

//////////////////////////////////////////////////////////////////////
//
// compile-max/capture
//   Factor capturing out when r is deterministic.
//      (r)* => (?:r*(r))?
//      (r)+ => (?:r*(r))
//  This xform exposes mergable nested loop.
static bool
compileMaxCapture(
    Compiler*       pCompiler,
    const LoopInfo* pLoop,
    int             iMinRest )
{
    unless (pLoop->m_iMin <= 1 && pLoop->m_iMax == Infinity)
    {
        return false;
    }

    NodeCapture* pNodeCapture = pLoop->node_->DynamicCast<NodeCapture>();
    if (NULL == pNodeCapture)
    {
        return false;
    }

    LengthInfo oLength = pNodeCapture->ComputeLength();
    unless (oLength.IsFixed())
    {
        return false;
    }

    Node* pNode = new(pCompiler->GetHeap()) NodeAnd(
        new(pCompiler->GetHeap()) NodeMax(
            pNodeCapture->GetDirection(),
            pNodeCapture->GetNode(),
            MinMax(0, Infinity) ),
        pNodeCapture );
    if (0 == pLoop->m_iMin)
    {
        pNode = new(pCompiler->GetHeap()) NodeMax(
            pNodeCapture->GetDirection(),
            pNode,
            MinMax(0, 1) );
    }

    pNode->Compile(pCompiler, iMinRest);
    return true;
} // compileMaxCapture

//////////////////////////////////////////////////////////////////////
//
// compile-loop
//  Compiles greedy, lazzy and posessive (NYI) repeation.
//
//  Compiled form of repetition "r*" has following form:
//           OR       L001          ; when min == 0
//           PUSH-INT -min          ; push initial value of loop counter
//      L002 PUSH-POS               ; push pos for null match check
//           ... r ...
//           NULL     L001          ; check null match
//           REST     n             ; check rest length
//           MAX      L002 max-min  ; check and increment loop counter
//      L001 ...
//
// BUGBUG: We should count min-rest of WHEN and UNLESS, since
// regex-study-length returns 0 for them.
//
static void
compileLoop(
    Compiler*       pCompiler,
    NodeMinMax*     pNode,
    const LoopInfo* pLoop,
    int             nMinRest )
{
    int  nMinSubLen = pLoop->node_->ComputeMinLength();
    bool fMatchNull = 0 == nMinSubLen;
    bool fCounter   = pLoop->m_iMin >= 2 || pLoop->m_iMax != Infinity;
    int  iPatch     = 0;

    if (0 == pLoop->m_iMin)
    {
        if (pNode->Is<NodeMax>())
        {
            iPatch = pCompiler->EmitRefLabel(Op_Or, 0);
        }
        else
        {
            iPatch = pCompiler->EmitRefLabel(Op_Push, 0);
        }
    }

    if (fCounter)
    {
        pCompiler->Emit(Op_PushInt, -pLoop->m_iMin);
    }

    int iLoopPc = pCompiler->GetPc();
    if (fMatchNull)
    {
        pCompiler->Emit(Op_PushPosn);
    }

    pCompiler->StartLoop();
    pLoop->node_->Compile(pCompiler, nMinRest);
    pCompiler->EndLoop();

    if (fMatchNull)
    {
        if (fCounter)
        {
            iPatch = pCompiler->EmitRefLabel(Op_Nulc, iPatch);
        }
        else
        {
            iPatch = pCompiler->EmitRefLabel(Op_Null, iPatch);
        }
    }

    if (0 == nMinRest)
    {
        // nothing to do
    }
    else if (pCompiler->IsInLoop())
    {
        pCompiler->Emit(
            pNode->IsBackward() ? Op_Rest_B : Op_Rest_F,
            nMinRest );
    }
    else
    {
        // We use Op_Last for the last outermost loop.
        pCompiler->PatchOpLast();

        pCompiler->Emit(
            pNode->IsBackward() ? Op_Last_B : Op_Last_F,
            nMinRest );
    } // if

    if (fCounter)
    {
        pCompiler->Emit(
            pNode->Is<NodeMax>() ? Op_Max : Op_Min,
            iLoopPc,
            pLoop->m_iMax - pLoop->m_iMin );
    }
    else
    {
        pCompiler->Emit(
            pNode->Is<NodeMax>() ? Op_Push : Op_Or,
            iLoopPc );
    }

    pCompiler->PatchLabels(iPatch);
} // compileLoop

//////////////////////////////////////////////////////////////////////
//
// Compiler::CompileMaxFixed
//
// For a{4} and a{4}?
// FIXME 2007-08-09 NYI: We should use min-rest for compile-max/fixed.
bool
Compiler::CompileMaxFixed(
    int     nN,
    Node*   pNode,
    int     nMinRest )
{
    if (nN <= 0)
    {
        return false;
    }

    if (NodeAny* pAny = pNode->DynamicCast<NodeAny>())
    {
        if (nN <= 10)
        {
            if (pAny->IsBackward())
            {
                for (int i = 0; i < nN; i++)
                {
                    Emit(Op_Any_B);
                } // for
            }
            else
            {
                for (int i = 0; i < nN; i++)
                {
                    Emit(Op_Any_F);
                } // for
            }
            return true;
        } // if
        return false;
    } // if

    // c{1,1}  => c
    // c{1,k}   => c....c  when k<= 20
    if (NodeChar* pChar = pNode->DynamicCast<NodeChar>())
    {
        if (1 == nN)
        {
            pChar->Compile(this, nMinRest);
            return true;
        }

        if (nN <= 20)
        {
            CompilerString* pString =
                new(m_pHeap) CompilerString(m_pHeap, nN);

            fillChar(pString->Get(), pChar->GetChar(), nN);

            Emit(
                computeStringOp(
                    pChar->IsBackward(),
                    pChar->IsIgnoreCase(),
                    pChar->IsNot() ),
                pString );
            return true;
        } // if n <= 20
        return false;
    } // if char

    // r{1,k} => r...r when k <= 30 and |r|=1
    if (pNode->Is<NodeOneWidth>())
    {
        if (nN <= 30)
        {
            for (int i = 0; i < nN; i++)
            {
                pNode->Compile(this, nMinRest);
                nMinRest -= 1;
            } // for
            return true;
        } // if
        return false;
    } // if onewidth

    return false;
} // CompileMaxFixed

//////////////////////////////////////////////////////////////////////
//
// Compiler::CompileMaxSimple
//
//  Processes following simple regex infinite repetaions:
//      .       => Repeat Any
//      <char>  => Repeat Character
//      [aiueo] => Repeat CharSet
//      [a-z]   => Repeat Range
//      \d      => Repeat One Width (\d \s \w \D \S \W)
bool
Compiler::CompileMaxSimple(
    const LoopInfo* pLoop,
    int             nMinRest )
{
    if (pLoop->m_iMax != Infinity)
    {
        return false;
    }

    Node* pNode = pLoop->node_;

    if (NodeAny* pAny = pNode->DynamicCast<NodeAny>())
    {
        switch (pLoop->m_iMin)
        {
        case 0:
            Emit(Op_Or, GetPc() + 4);
            break;

        case 1:
            break;

        default:
            if (! CompileMaxFixed(pLoop->m_iMin - 1, pAny, nMinRest))
            {
                return false;
            }
            break;
        } // switch min

        Emit(
            pAny->IsBackward() ? Op_RepeatAny_B : Op_RepeatAny_F,
            nMinRest );

        return true;
    } // if NodeAny

    if (NodeChar* pChar = pNode->DynamicCast<NodeChar>())
    {
        switch (pLoop->m_iMin)
        {
        case 0:
            Emit(Op_Or, GetPc() + 5);
            break;

        case 1:
            break;

        default:
            if (! CompileMaxFixed(pLoop->m_iMin - 1, pNode, nMinRest))
            {
                return false;
            }
            break;
        } // switch min

        Emit(
            computeRepeatCharOp(pChar),
            nMinRest,
            pChar->GetChar() );

        return true;
    } // if NodeChar

    // For /[aiueo]{n,}/
    {
        NodeCharSet* pCharSet = pNode->DynamicCast<NodeCharSet>();
        if (NULL != pCharSet)
        {
            if (0 == pLoop->m_iMin) Emit(Op_Or, GetPc() + 5);

            Emit(
                computeRepeatCharSetOp(pCharSet),
                nMinRest,
                CreateString(
                    pCharSet->GetString(),
                    pCharSet->GetLength() ) );

            return true;
        } // if
    } // NodeCharSet

    // For /[cc]{n,}/
    {
        NodeRange* pRange = pNode->DynamicCast<NodeRange>();

        if (NULL != pRange)
        {
            if (pRange->IsIgnoreCase()) return false;

            if (0 == pLoop->m_iMin) Emit(Op_Or, GetPc() + 6);

            Emit(
                computeRepeatRangeOp(pRange),
                nMinRest,
                pRange->GetMinChar(),
                pRange->GetMaxChar() );

            return true;
        } // if
    } // Range

    // For /\d{n,}/
    {
        NodeOneWidth* pOneWidth = pNode->DynamicCast<NodeOneWidth>();

        if (NULL != pOneWidth)
        {
            if (0 == pLoop->m_iMin) Emit(Op_Or, GetPc() + 4);

            Emit(
                computeRepeatOneWidthOp(pOneWidth->GetOp()),
                nMinRest );

            return true;
        } // if
    } // OneWidth

    return false;
} // Compiler::CompileMaxSimple

void NodeMax::Compile(Compiler* pCompiler, int nMinRest)
{
    LoopInfo oLoop = foldNestedLoop(this, GetMin(), GetMax(), GetNode());
    if (0 == oLoop.m_iMin && 0 == oLoop.m_iMax) return;

    if (0 == oLoop.m_iMin && 1 == oLoop.m_iMax)
    {
        // For r? == r{0,1}
        int iPushPc = pCompiler->EmitRefLabel(Op_Or);
        GetNode()->Compile(pCompiler, nMinRest);
        pCompiler->PatchLabel(iPushPc);
        return;
    }

    if (oLoop.m_iMin == oLoop.m_iMax)
    {
        if (pCompiler->CompileMaxFixed(oLoop.m_iMin, oLoop.node_, nMinRest))
        {
            return;
        }
    }

    if (pCompiler->CompileMaxSimple(&oLoop, nMinRest)) return;
    if (compileMaxCapture(pCompiler, &oLoop, nMinRest)) return;

    compileLoop(pCompiler, this, &oLoop, nMinRest);
} // NodeMax::Compile

void NodeMin::Compile(Compiler* pCompiler, int nMinRest)
{
    LoopInfo oLoop = foldNestedLoop(
        this,
        GetMin(),
        GetMax(),
        GetNode() );
    if (0 == oLoop.m_iMin && 0 == oLoop.m_iMax)
    {
        return;
    }

    if (0 == oLoop.m_iMin && 1 == oLoop.m_iMax)
    {
        int iPushPc = pCompiler->EmitRefLabel(Op_Push);
        GetNode()->Compile(pCompiler, nMinRest);
        pCompiler->PatchLabel(iPushPc);
        return;
    }

    if (oLoop.m_iMin == oLoop.m_iMax)
    {
        if (pCompiler->CompileMaxFixed(oLoop.m_iMin, GetNode(), nMinRest))
        {
            return;
        }
    } // if

    compileLoop(pCompiler, this, &oLoop, nMinRest);
} // NodeMin::Compile

void NodeOneWidth::CompileNot(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetNotOp());
} // NodeOneWidth::CompileNot

void NodeOpBase::Compile(Compiler* pCompiler, int)
{
    pCompiler->Emit(GetOp());
} // NodeOpBase::Compile

//////////////////////////////////////////////////////////////////////
//
// NodeOr::Compile
//
//  (or)        => nothing
//  (or Node)   => Node
//  (or char* Node char*)   => (or (union char*) Node (union char*))
//
//      OR L_1
//      ... r1 ...
//      GO L_end
//  L_1:
//      OR L_2
//      ... r2 ...
//      GO L_end
//  L_2:
//      ...
//  L_end:
//      ...
void NodeOr::Compile(Compiler* pCompiler, int nMinRest)
{
    if (NULL == m_oNodes.GetFirst())
    {
        // (or) => nothing (always succeeded)
        return;
    }

    if (NULL == m_oNodes.GetFirst()->GetNext())
    {
        // (or re) => re
        m_oNodes.GetFirst()->Compile(pCompiler, nMinRest);
        return;
    }

    int nEndPc = 0;
    foreach (Nodes::Enum, oEnum, &m_oNodes)
    {
        Node* pNode = oEnum.Get();

        int nPushPc = 0;
        if (NULL != pNode->GetNext())
        {
            // FIXME 2007-08-11 yosi@msn.com NYI emit pop if all altanatives
            // are distinct.
            nPushPc = pCompiler->EmitRefLabel(Op_Or, nPushPc);
        }

        pNode->Compile(pCompiler, nMinRest);

        if (0 != nPushPc)
        {
            nEndPc = pCompiler->EmitRefLabel(Op_Go, nEndPc);
            pCompiler->PatchLabel(nPushPc);
        }
    } // for each Node

    pCompiler->PatchLabels(nEndPc);
} // NodeOr::Compile

void NodeRange::Compile(Compiler* pCompiler, int)
{
    pCompiler->CompileRange(this, IsNot());
} // NodeRange::Compile

void NodeRange::CompileNot(Compiler* pCompiler, int)
{
    pCompiler->CompileRange(this, ! IsNot());
} // NodeRange::CompileNot

void Compiler::CompileRange(const NodeRange* pRange, bool fNot)
{
    char16 wchMin = pRange->GetMinChar();
    char16 wchMax = pRange->GetMaxChar();
    if (wchMin == wchMax)
    {
        int iOp  = pRange->GetOp(fNot);
            iOp += Op_RepeatCharEq_Ci_B - Op_RangeEq_Ci_B;
        Emit(static_cast<Op>(iOp), wchMin);
    }
    else
    {
        Emit(pRange->GetOp(fNot), wchMin, wchMax);
    }
} // NodeRange::Compile

void NodeString::Compile(Compiler* pCompiler, int)
{
    CompilerString* pString = pCompiler->CreateString(m_pwch, m_cwch);
    pCompiler->Emit(
        GetOp(IsNot()),
        pString );
} // NodeString::Compile

RegexObj*
CompileRegex(ICompileContext* pIContext, LocalHeap* pHeap, Tree* pTree)
{
    Compiler oCompiler(pIContext, pHeap);
    return oCompiler.Compile(pTree);
} // CompileRegex

} // RegexPrivate
} // Regex
