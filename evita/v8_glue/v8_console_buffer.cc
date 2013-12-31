// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/v8_console_buffer.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/ap_input_history.h"
#include "evita/cm_CmdProc.h"
#include "evita/dom/script_thread.h"
#include "evita/editor/dom_lock.h"
#include "evita/ed_Range.h"
#include "evita/vi_Selection.h"

namespace v8_glue {

namespace {
Command::KeyBinds* s_key_bindings;

}  // namespace

V8ConsoleBuffer::V8ConsoleBuffer()
    : Buffer(L"*javascript console*", nullptr),
      input_history_(new InputHistory()),
      prompt_end_(CreateRange()),
      script_end_(CreateRange()) {
  SetNoSave(true);
  PopulateKeyBindings();
  Emit("// JavaScript Console for V8 ");
  Emit(v8::V8::GetVersion());
  EmitPrompt();
}

V8ConsoleBuffer::~V8ConsoleBuffer() {
}

void V8ConsoleBuffer::Emit(const base::StringPiece& pice) {
  Emit(base::ASCIIToUTF16(pice));
}

void V8ConsoleBuffer::Emit(const base::string16& string) {
  SetReadOnly(false);
  Insert(GetEnd(), string.data(), string.length());
  SetReadOnly(true);
}

void V8ConsoleBuffer::Emit(int value) {
  Emit(base::IntToString16(value));
}

void V8ConsoleBuffer::EmitPrompt() {
  Emit("\n> ");
  prompt_end_->SetRange(GetEnd(), GetEnd());
  SetReadOnly(false);
  for (auto& window: windows()) {
    auto const selection = window.GetSelection();
    if (selection->GetStart() == script_end_->GetStart() &&
        selection->GetEnd() == script_end_->GetEnd()) {
      selection->SetRange(GetEnd(), GetEnd());
    }
  }
}

void V8ConsoleBuffer::EnterCommand(const Command::Context* context) {
  auto& selection = *context->GetSelection();
  auto& v8_buffer = *static_cast<V8ConsoleBuffer*>(selection.GetBuffer());
  v8_buffer.ExecuteLastLine();
}

void V8ConsoleBuffer::ExecuteLastLine() {
  SetReadOnly(true);
  auto const script_start = prompt_end_->GetEnd();
  auto const script_end = GetEnd();
  script_end_->SetRange(script_end, script_end);
  input_history_->Add(this, script_start, script_end);
  Emit("\n");
  StyleValues style;
  style.m_rgfMask = StyleValues::Mask_Color | StyleValues::Mask_FontStyle;
  style.m_crColor = RGB(0x55, 0x00, 0x00);
  style.m_eFontStyle = FontStyle_Italic;
  SetStyle(script_start, script_end, &style);

  base::string16 text(script_end - script_start, ' ');
  GetText(&text[0], script_start, script_end);

  // TODO(yosi) We should retain |this| until |HandleEvaluateResult|.
  dom::ScriptThread::instance()->Evaluate(
      text, base::Bind(&V8ConsoleBuffer::HandleEvaluateResult,
                       base::Unretained(this)));
}

void V8ConsoleBuffer::HandleEvaluateResult(dom::EvaluateResult result) {
  UI_DOM_AUTO_LOCK_SCOPE();
  if (result.exception.empty()) {
    Emit(result.value);
  } else if (result.script_resource_name.empty()) {
    Emit(result.exception);
  } else {
    // Print (filename):(line number): (message)\n
    Emit(result.script_resource_name);
    Emit(":");
    Emit(result.line_number);
    Emit(": ");
    Emit(result.exception);
    Emit("\n");
    // Print line of sourc code
    auto const line_start = GetEnd();
    Emit(result.source_line);
    Emit("\n");

    StyleValues style;
    style.m_rgfMask = StyleValues::Mask_Color | StyleValues::Mask_FontWeight;
    style.m_crColor = RGB(0xFF, 0x00, 0x00);
    style.m_eFontWeight = FontWeight_Bold;
    SetStyle(line_start + result.start_column, line_start + result.end_column,
             &style);
    for (auto line: result.stack_trace) {
      Emit(line);
      Emit("\n");
    }
  }
  EmitPrompt();
}

Command::KeyBindEntry* V8ConsoleBuffer::MapKey(uint key_code) const {
  auto const entry = s_key_bindings->MapKey(key_code);
  return entry ? entry : Command::g_pGlobalBinds->MapKey(key_code);
}

void V8ConsoleBuffer::PopulateKeyBindings() {
  if (s_key_bindings)
    return;
  s_key_bindings = new Command::KeyBinds();
  s_key_bindings->Bind(Command::MapVKey(VK_RETURN),
                       new Command::Command(EnterCommand));
}

}  // namespace v8_glue
