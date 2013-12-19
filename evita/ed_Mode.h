//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode
// listener/winapp/ed_Mode.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Mode.h#8 $
//
#if !defined(INCLUDE_edit_Mode_h)
#define INCLUDE_edit_Mode_h

#include "./ed_Buffer.h"

#include "./li_util.h"

#include "./vi_Application.h"

namespace Edit
{

typedef HashTable_<StringKey, int, 100> KeywordTable;

/// <summary>
///   Character syntax definition
/// </summary>
class CharSyntax
{
    public: enum Syntax
    {
        Syntax_None         = 0x0,

        Syntax_CloseParen   = 0x1,
        Syntax_Control      = 0x2,
        Syntax_Escape       = 0x3,
        Syntax_LineComment  = 0x4,
        Syntax_OpenParen    = 0x5,
        Syntax_Punctuation  = 0x6,
        Syntax_StringQuote  = 0x7,
        Syntax_Whitespace   = 0x8,
        Syntax_Word         = 0x9,
    }; // Syntax

    enum Constant
    {
        Trait_PairShift    = 4,
        Trait_SyntaxMask   = (1 << Trait_PairShift) - 1,

        Trait_BlockCommentEnd1     = 0x0001 << 16,
        Trait_BlockCommentEnd2     = 0x0002 << 16,
        Trait_BlockCommentEnd      = 0x0003 << 16,
        Trait_BlockCommentStart1   = 0x0010 << 16,
        Trait_BlockCommentStart2   = 0x0020 << 16,
        Trait_BlockCommentStart    = 0x0030 << 16,
        Trait_LineCommentStart     = 0x0100 << 16,
    }; // Constant

    public: static char16 GetPairChar(uint nSyntax)
    {
        return static_cast<char16>((nSyntax >> Trait_PairShift) & 0xFF);
    } // GetPairChar

    public: static Syntax GetSyntax(uint nSyntax)
    {
        return static_cast<Syntax>(nSyntax & Trait_SyntaxMask);
    } // GetSyntax
}; // CharSyntax

/// <summary>
///   Base class of coloring lexer
/// </summary>
class LexerBase
{
    protected: Posn                     m_lTokenStart;
    protected: Buffer::ChangeTracker    m_oChange;
    protected: Buffer::EnumChar         m_oEnumChar;
    protected: Buffer*                  m_pBuffer;

    // ctor/dtor
    protected: LexerBase(Buffer*);
    protected: ~LexerBase();
}; // LexerBase

namespace NewLexer
{

/// <summary>
///   Base class of coloring lexer
/// </summary>
class LexerBase
{
    private: class EnumChar : public Buffer::EnumChar
    {
        private: typedef Buffer::EnumChar Super;

        private: Count m_lCount;

        public: EnumChar(Buffer* p) : Super (p) {}

        public: bool AtLimit() const
        {
            if (m_lCount <= 0)
            {
                return true;
            }

            return AtEnd();
        } // AtLimit

        public: char16 Get() const
        {
            ASSERT(! AtLimit());
            return Super::Get();
        } // GetChar

        public: void Next()
        {
            ASSERT(! AtLimit());
            m_lCount -= 1;
            Super::Next();
        } // Next

        public: void SetCounter(Count lCount)
            { m_lCount = lCount; }
    }; // EnumChar

    protected: Buffer::ChangeTracker    m_oChange;
    protected: EnumChar                 m_oEnumChar;
    protected: Buffer*                  m_pBuffer;
    protected: const uint*              m_prgnCharSyntax;
    protected: KeywordTable*            m_pKeywordTab;

    // ctor/dtor
    protected: LexerBase(Buffer*, KeywordTable*, const uint*);
    protected: ~LexerBase();

    // [A]
    protected: static void addKeywords(
        KeywordTable*,
        const char16**,
        size_t );

    // [I]
    protected: static KeywordTable* installKeywords(
        const char16**,
        size_t );

    /// <summary>
    ///   Returns true if specified character is constitute character.
    /// </summary>
    protected: bool isConsChar(char16 wch) const
    {
        if (wch >= 0x7F) return true;
        if (wch <= 0x20) return false;
        uint nSyntax = m_prgnCharSyntax[wch - 0x20];
        return CharSyntax::Syntax_Word == CharSyntax::GetSyntax(nSyntax);
    } // isConsChar

    /// <summary>
    ///   Returns true if specified character is punctuation character.
    /// </summary>
    protected: bool isPunctChar(char16 wch) const
    {
        if (wch >= 0x7F) return true;
        if (wch <= 0x20) return false;
        uint nSyntax = m_prgnCharSyntax[wch - 0x20];
        return CharSyntax::Syntax_Punctuation == CharSyntax::GetSyntax(nSyntax);
    } // isPunctChar

    // [S]
    protected: void setColor(
        Posn    lStart,
        Posn    lEnd,
        uint    nSyntax,
        uint32  nColor )
    {
        if (lStart >= lEnd)
        {
            return;
        }

        StyleValues oStyleValues;

        oStyleValues.m_rgfMask =
            StyleValues::Mask_Background |
            StyleValues::Mask_Color |
            StyleValues::Mask_Syntax;

        oStyleValues.m_crBackground = RGB(255, 255, 255);
        oStyleValues.m_crColor      = nColor;
        oStyleValues.m_nSyntax      = static_cast<int>(nSyntax);

        m_pBuffer->SetStyle(lStart, lEnd, &oStyleValues);
    } // setColor
}; // LexerBase

} // NewLexer

class ModeFactory;

/// <remark>
///   Edit mode factory
/// </remark>
/// <seealso cref="Mode"/>
class ModeFactory : public DoubleLinkedNode_<ModeFactory>
{
    protected: int          m_iIcon;
    protected: const uint*  m_prgnCharSyntax;

    // ctor
    public: ModeFactory(const uint* prgnCharSyntax) :
        m_iIcon(-1),
        m_prgnCharSyntax(prgnCharSyntax) {}

    // [C]
    public: virtual Mode* Create(Buffer*) = 0;

    // [E]
    protected: virtual const char16* getExtensions() const
        { return L""; }

    // [G]
    public: virtual const char16* GetName() const = 0;
    public: static ModeFactory* Get(Buffer*);
    public: uint GetCharSyntax(char16) const;

    // [I]
    public: virtual bool IsSupported(const char16*) const;
}; // ModeFactory

typedef DoubleLinkedList_<ModeFactory> ModeFactoryes;

/// <remark>
///   Edit mode
/// </remark>
class Mode
{
    protected: int          m_iIcon;
    protected: Buffer*      m_pBuffer;
    protected: ModeFactory* m_pClass;

    // ctor/dtor
    protected: Mode(ModeFactory*, Buffer*);
    public: virtual ~Mode();

    // [D]
    public: virtual bool DoColor(Count) = 0;

    // [G]
    public: Buffer* GetBuffer() const
        { return m_pBuffer; }

    public: ModeFactory* GetClass() const
        { return m_pClass; }

    public: uint GetCharSyntax(char16 wch) const
        { return m_pClass->GetCharSyntax(wch); }

    public: virtual int GetIcon() const;

    public: virtual const char16* GetName() const
        { return m_pClass->GetName(); }
}; // Mode

} // Edit

#endif //!defined(INCLUDE_edit_Mode_h)
