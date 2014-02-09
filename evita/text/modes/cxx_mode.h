//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - C++ Mode
// listener/winapp/mode_Cxx.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Cxx.h#7 $
//
#if !defined(INCLUDE_evita_text_modes_cxx_mode_h)
#define INCLUDE_evita_text_modes_cxx_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode_factory.h"
#include "evita/text/buffer.h"

namespace text
{

/// <summary>
///  C++ mode factory
/// </summary>
class CxxModeFactory : public common::Singleton<CxxModeFactory>,
                       public ModeFactory {
    DECLARE_SINGLETON_CLASS(CxxModeFactory);

    // ctor
    private: CxxModeFactory();
    public: ~CxxModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"cc cpp cxx c hpp hxx h css cs ev mm"; }

    public: virtual const char16* GetName() const override
        { return L"C++"; }

    DISALLOW_COPY_AND_ASSIGN(CxxModeFactory);
}; // CxxModeFactory

/// <summary>
///  Java mode factory
/// </summary>
class JavaModeFactory : public common::Singleton<JavaModeFactory>,
                        public ModeFactory {
    DECLARE_SINGLETON_CLASS(JavaModeFactory);

    // ctor
    private: JavaModeFactory();
    public: ~JavaModeFactory();

    // [C]
    public: virtual Mode* Create(Buffer*) override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"java js"; }

    public: virtual const char16* GetName() const override
        { return L"Java"; }

    DISALLOW_COPY_AND_ASSIGN(JavaModeFactory);
}; // JavaModeFactory

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_cxx_mode_h)
