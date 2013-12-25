// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/v8_console_buffer.h"

#include "base/strings/string16.h"
#include "evita/ap_input_history.h"
#include "evita/cm_CmdProc.h"
#include "evita/editor/application.h"
#include "evita/ed_Range.h"
#include "evita/vi_Selection.h"
BEGIN_V8_INCLUDE
#include "gin/object_template_builder.h"
END_V8_INCLUDE

namespace v8_glue {

namespace {
Command::KeyBinds* s_key_bindings;

class Initializer {
  public: static v8::Handle<v8::Context> CreateContext(v8::Isolate* isolate) {
    return v8::Context::New(isolate, nullptr, GetGlobalTemplate(isolate));
  }

  public: static v8::Isolate* CreateIsolate() {
    DEFINE_STATIC_LOCAL(bool, v8_initialized, (false));
    if (!v8_initialized) {
      v8_initialized = true;
      v8::V8::InitializeICU();
    }
    return v8::Isolate::GetCurrent();
  }

  private: static v8::Handle<v8::ObjectTemplate> GetGlobalTemplate(
      v8::Isolate* isolate) {
    DEFINE_STATIC_LOCAL(v8::Persistent<v8::ObjectTemplate>, global_template);
    if (!global_template.IsEmpty())
      return v8::Local<v8::ObjectTemplate>::New(isolate, global_template);

    gin::ObjectTemplateBuilder global_builder(isolate);

    {
      auto context = v8::Context::New(isolate);
      v8::Context::Scope context_scope(context);
      auto editor_builder = Application::instance()
          .GetObjectTemplateBuilder(isolate);
      auto editor = editor_builder.Build()->NewInstance();
      global_builder.SetValue("editor", v8::Handle<v8::Value>(editor));
    }
    global_template.Reset(isolate, global_builder.Build());
    return v8::Local<v8::ObjectTemplate>::New(isolate, global_template);
  }
};

}  // namespace

V8ConsoleBuffer::V8ConsoleBuffer()
    : Buffer(L"*javascript console*", nullptr),
      input_history_(new InputHistory()),
      context_holder_(isolate_holder_.isolate()),
      prompt_end_(CreateRange()) {
  SetNoSave(true);
  PopulateKeyBindings();
  isolate_holder_.isolate()->Enter();
  {
    v8::HandleScope handle_scope(isolate_holder_.isolate());
    auto context = Initializer::CreateContext(isolate_holder_.isolate());
    context_holder_.SetContext(context);
    context->Enter();
    Emit(L"// JavaScript Console for V8 version ");
    Emit(v8::String::NewFromUtf8(isolate_holder_.isolate(),
         v8::V8::GetVersion()));
  }
  EmitPrompt();
}

V8ConsoleBuffer::~V8ConsoleBuffer() {
  {
    v8::HandleScope handle_scope(isolate_holder_.isolate());
    context_holder_.context()->Exit();
  }
  isolate_holder_.isolate()->Exit();
}

void V8ConsoleBuffer::Emit(const v8::Handle<v8::Value>& value) {
  v8::String::Value string(value);
  if (!string.length())
    return;
  SetReadOnly(false);
  Insert(GetEnd(), reinterpret_cast<base::char16*>(*string), string.length());
  SetReadOnly(true);
}

void V8ConsoleBuffer::Emit(const base::char16* string) {
  SetReadOnly(false);
  Insert(GetEnd(), string);
  SetReadOnly(true);
}

void V8ConsoleBuffer::Emit(int value) {
  base::char16 buf[20];
  ::wsprintfW(buf, L"%i", value);
  Emit(buf);
}

void V8ConsoleBuffer::EmitPrompt() {
  Emit(L"\n> ");
  prompt_end_->SetRange(GetEnd(), GetEnd());
  SetReadOnly(false);
}

void V8ConsoleBuffer::EnterCommand(const Command::Context* context) {
  auto& selection = *context->GetSelection();
  auto& v8_buffer = *static_cast<V8ConsoleBuffer*>(selection.GetBuffer());
  v8_buffer.ExecuteLastLine();
  v8_buffer.EmitPrompt();
  selection.SetRange(v8_buffer.GetEnd(), v8_buffer.GetEnd());
}

void V8ConsoleBuffer::ExecuteLastLine() {
  SetReadOnly(true);
  auto const script_start = prompt_end_->GetEnd();
  auto const script_end = GetEnd();
  input_history_->Add(this, script_start, script_end);
  Emit(L"\n");
  StyleValues style;
  style.m_rgfMask = StyleValues::Mask_Color | StyleValues::Mask_FontStyle;
  style.m_crColor = RGB(0x55, 0x00, 0x00);
  style.m_eFontStyle = FontStyle_Italic;
  SetStyle(script_start, script_end, &style);

  base::string16 text(script_end - script_start, ' ');
  GetText(&text[0], script_start, script_end);

  v8::HandleScope handle_scope(isolate_holder_.isolate());
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(
      v8::String::NewFromTwoByte(isolate_holder_.isolate(),
                                 reinterpret_cast<uint16*>(&text[0]),
                                 v8::String::kNormalString,
                                 text.size()),
      v8::String::NewFromUtf8(isolate_holder_.isolate(), "(console)"));
   if (script.IsEmpty()) {
     ReportException(&try_catch);
     return;
   }
   v8::Handle<v8::Value> result = script->Run();
   if (result.IsEmpty()) {
     DCHECK(try_catch.HasCaught());
     ReportException(&try_catch);
     return;
   }
   DCHECK(!try_catch.HasCaught());
   if (result->IsUndefined())
     return;
   Emit(result);
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

void V8ConsoleBuffer::ReportException(v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate_holder_.isolate());
  v8::Handle<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    Emit(try_catch->Exception());
    return;
  }
  // Print (filename):(line number): (message)\n
  Emit(message->GetScriptResourceName());
  Emit(L":");
  Emit(message->GetLineNumber());
  Emit(L": ");
  Emit(try_catch->Exception());
  Emit(L"\n");
  // Print line of sourc code
  auto const line_start = GetEnd();
  Emit(message->GetSourceLine());
  Emit(L"\n");

  StyleValues style;
  style.m_rgfMask = StyleValues::Mask_Color | StyleValues::Mask_FontWeight;
  style.m_crColor = RGB(0xFF, 0x00, 0x00);
  style.m_eFontWeight = FontWeight_Bold;
  SetStyle(line_start + message->GetStartColumn(),
           line_start + message->GetEndColumn(), &style);

  if (!try_catch->StackTrace()->IsUndefined()) {
    Emit(try_catch->StackTrace());
    Emit(L"\n");
  }
}

}  // namespace v8_glue
