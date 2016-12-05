// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ginx/ginx_util.h"

#include "base/logging.h"
#include "gin/converter.h"

namespace ginx {

bool IsInstanceOf(v8::Local<v8::Context> context,
                  v8::Local<v8::Object> object,
                  v8::Local<v8::Object> clazz) {
  DCHECK(clazz->IsConstructor());
  auto* const isolate = context->GetIsolate();
  auto constructorKey = gin::StringToV8(isolate, "constructor");
  for (v8::Local<v8::Value> runner = object; runner->IsObject();
       runner = runner->ToObject()->GetPrototype()) {
    auto constructor = runner->ToObject(context)
                           .ToLocalChecked()
                           ->Get(context, constructorKey)
                           .FromMaybe(v8::Local<v8::Value>());
    if (constructor == clazz)
      return true;
  }
  return false;
}

void ImproveErrorMessage(v8::Local<v8::Context> context,
                         const v8::TryCatch& try_catch) {
  auto const message = try_catch.Message();
  if (message.IsEmpty())
    return;
  auto const js_error = try_catch.Exception()->ToObject(context).FromMaybe(
      v8::Local<v8::Object>());
  if (js_error.IsEmpty())
    return;
  auto* const isolate = context->GetIsolate();
  auto const error_class = context->Global()
                               ->Get(context, gin::StringToV8(isolate, "Error"))
                               .ToLocalChecked()
                               ->ToObject(context)
                               .FromMaybe(v8::Local<v8::Object>());
  if (!IsInstanceOf(context, js_error, error_class))
    return;

  auto const fileNameKey = gin::StringToV8(isolate, "fileName");
  if (!js_error->Has(context, fileNameKey).FromMaybe(false))
    js_error->Set(fileNameKey, message->GetScriptResourceName());

  auto const sourceLineKey = gin::StringToV8(isolate, "sourceLine");
  if (!js_error->Has(context, sourceLineKey).FromMaybe(false)) {
    auto sourceLine =
        message->GetSourceLine(context).FromMaybe(v8::Local<v8::String>());
    if (!sourceLine.IsEmpty())
      js_error->Set(gin::StringToV8(isolate, "sourceLine"), sourceLine);
  }

  auto const lineNumberKey = gin::StringToV8(isolate, "lineNumber");
  if (!js_error->Has(context, lineNumberKey).FromMaybe(false)) {
    const int lineNumber = message->GetLineNumber(context).FromMaybe(0);
    js_error->Set(gin::StringToV8(isolate, "lineNumber"),
                  gin::ConvertToV8(isolate, lineNumber));
  }

  auto const columnNumberKey = gin::StringToV8(isolate, "columnNumber");
  if (!js_error->Has(context, columnNumberKey).FromMaybe(false)) {
    const int columnNumber = message->GetStartColumn(context).FromMaybe(0);
    js_error->Set(gin::StringToV8(isolate, "columnNumber"),
                  gin::ConvertToV8(isolate, columnNumber));
  }
}

}  // namespace ginx
