//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - XML Mode
// listener/winapp/mode_Xml.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Xml.h#3 $
//
#if !defined(INCLUDE_mode_Xml_h)
#define INCLUDE_mode_Xml_h

#include "./ed_Mode.h"

namespace Edit
{

//////////////////////////////////////////////////////////////////////
//
// XmlLexer
//
class XmlLexer : public LexerBase
{
    public: enum State
    {
        State_Error,

        State_Text,
        State_Lt,
        State_Amp,

        State_Tag,                         // < Name
            State_Att,
            State_AttVal1,
            State_AttVal2,

        State_Lt_Bang,                         // <!
            State_Lt_Bang_Dash,                // <!-
                State_Comment,
                    State_Comment_Dash,        // <!-- ... -
                    State_Comment_Dash_Dash,   // <!-- ... --

            State_Lt_Bang_LBrk,                // <![
                State_Lt_Bang_LBrk_C,
                State_Lt_Bang_LBrk_CD,
                State_Lt_Bang_LBrk_CDA,
                State_Lt_Bang_LBrk_CDAT,
                State_Lt_Bang_LBrk_CDATA,
                    State_CData,               // <![CDATA[...
                    State_CData_RBrk,          // <![CDATA[... ]
                    State_CData_RBrk_RBrk,     // <![CDATA[... ]]

        State_Limit,
    }; // State

    enum Style
    {
        Style_Unknown,
        Style_Keyword,
        Style_AttVal1,
        Style_AttVal2,
        Style_Comment,
        Style_EntityRef,
        Style_Tag,
        Style_Text,
        Style_CData,
        Style_Sym,
        Style_Att,

        Style_Limit,
    }; // enum StyleT

    private: State m_eState;

    // ctor
    public: XmlLexer(Buffer*);

    // [P]
    private: void processTag();

    // [R]
    private: void restart();
    public: bool  Run(Count);
    private: void runAux(char16);

    // [S]
    private: void setColor(State, int = 0);
    private: void setStateCont(State);
    private: void setStateEnd(State, int = 0);
    private: void setStateStart(State, int = 0);
}; // XmlLexer


//////////////////////////////////////////////////////////////////////
//
// XmlMode
//
class XmlMode : public Mode
{
    private: XmlLexer m_oLexer;

    // ctor
    public: XmlMode(ModeFactory*, Buffer*);

    // [D]
    public: virtual bool DoColor(Count) override;
}; // XmlMode

/// <summary>
///   XML Mode factory
/// </summary>
class XmlModeFactory : public ModeFactory
{
    // ctor
    public: XmlModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer* pBuffer) override
        { return new XmlMode(this, pBuffer); }

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"xml xsl xsd xhtml html htm wsdl asdl"; }

    public: virtual const char16* GetName() const override
        { return L"XML"; }
}; // XmlModeFactory

} // Edit

#endif //!defined(INCLUDE_mode_Xml_h)
