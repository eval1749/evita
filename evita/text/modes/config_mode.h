//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - C++ Mode
// listener/winapp/mode_Config.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Config.h#3 $
//
#if !defined(INCLUDE_evita_text_modes_config_mode_h)
#define INCLUDE_evita_text_modes_config_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text {

/// <summary>
///   Configuration file mode
/// </summary>
class ConfigModeFactory : public common::Singleton<ConfigModeFactory>,
                          public ModeFactory {
    DECLARE_SINGLETON_CLASS(ConfigModeFactory);

    // ctor
    private: ConfigModeFactory();
    public: ~ConfigModeFactory();

    // [C]
    public: virtual Mode* Create() override;

    // [G]
    protected: virtual const char16* getExtensions() const override
        { return L"cfg mk stanza"; }

    public: virtual const char16* GetName() const override
        { return L"Config"; }

    // [I]
    public: virtual bool IsSupported(const char16*) const override;

    DISALLOW_COPY_AND_ASSIGN(ConfigModeFactory);
}; // ConfigModeFactory

class ConfigMode : public ModeWithLexer {
  public: ConfigMode();
  public: virtual ~ConfigMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(ConfigMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_config_mode_h)
