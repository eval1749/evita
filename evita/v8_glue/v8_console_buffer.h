// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_v8_console_buffer_h)
#define INCLUDE_evita_v8_glue_v8_console_buffer_h

#include <memory>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/vi_Buffer.h"
#include "evita/v8_glue/context_holder.h"

class InputHistory;

namespace Command {
class Context;
class KeyBindEntry;
}

namespace v8 {
class Context;
template<typename T> class Handle;
class Isolate;
} // namespace v8

namespace v8_glue {

class V8ConsoleBuffer : public common::Singleton<V8ConsoleBuffer>,
                        public Buffer {
  private: std::unique_ptr<InputHistory> input_history_;
  private: v8::Isolate* const isolate_;
  private: ContextHolder context_holder_;
  private: Edit::Range* prompt_end_;

  public: V8ConsoleBuffer();
  public: virtual ~V8ConsoleBuffer();

  private: void Emit(const v8::Handle<v8::Value>& value);
  private: void Emit(const base::char16* string);
  private: void Emit(int value);
  private: void EmitPrompt();
  private: static void EnterCommand(const Command::Context* context);
  private: void ExecuteLastLine();
  private: virtual Command::KeyBindEntry* MapKey(uint) const override;
  private: void PopulateKeyBindings();
  private: void ReportException(v8::TryCatch*);

  DISALLOW_COPY_AND_ASSIGN(V8ConsoleBuffer);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_v8_console_buffer_h)
