// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_modes_mode_h)
#define INCLUDE_evita_text_modes_mode_h

#include <memory>

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace text {

class Buffer;
class Lexer;
class Mode;
class ModeFactory;

class Mode {
  private: Buffer* buffer_;

  protected: Mode();
  public: virtual ~Mode();

  public: Buffer* buffer() const { return buffer_; }
  public: void set_buffer(Buffer* buffer);

  public: virtual bool DoColor(Count) = 0;
  public: virtual const char16* GetName() const = 0;

  DISALLOW_COPY_AND_ASSIGN(Mode);
};

class ModeWithLexer : public Mode {
  private: std::unique_ptr<Lexer> lexer_;

  protected: ModeWithLexer();
  protected: virtual ~ModeWithLexer();

  protected: virtual Lexer* CreateLexer(Buffer* buffer) = 0;

  // Mode
  private: virtual bool DoColor(Count) override;

  DISALLOW_COPY_AND_ASSIGN(ModeWithLexer);
};

} // namespace text

#endif //!defined(INCLUDE_evita_text_modes_mode_h)
