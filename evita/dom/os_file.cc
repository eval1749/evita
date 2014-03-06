// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os_file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/os/file_handle.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/object_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "v8_strings.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// OpenFileCallback
//
class OpenFileCallback : public base::RefCounted<OpenFileCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: OpenFileCallback(v8_glue::Runner* runner,
                          v8::Handle<v8::Function> function)
    : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: void Run(domapi::IoHandle* handle, int error_code) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    if (error_code) {
      runner_->Call(function, v8::Undefined(isolate),
                    v8::Integer::New(isolate, error_code));
    } else {
      runner_->Call(function, v8::Undefined(isolate),
                    (new FileHandle(handle))->GetWrapper(isolate));
    }
  }

  DISALLOW_COPY_AND_ASSIGN(OpenFileCallback);
};

void OpenFile(const base::string16& file_name, const base::string16& mode,
              v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const file_io_callback = make_scoped_refptr(
      new OpenFileCallback(runner, callback));
  ScriptController::instance()->io_delegate()->OpenFile(file_name, mode,
      base::Bind(&OpenFileCallback::Run, file_io_callback));
}

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusCallback
//
class QueryFileStatusCallback
    : public base::RefCounted<QueryFileStatusCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: QueryFileStatusCallback(v8_glue::Runner* runner,
                                  v8::Handle<v8::Function> function)
      : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: ~QueryFileStatusCallback() = default;

  public: void Run(const domapi::QueryFileStatusCallbackData& data) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    auto const js_data = v8::Object::New(isolate);
    js_data->Set(v8Strings::errorCode.Get(isolate),
                 gin::ConvertToV8(isolate, data.error_code));
    js_data->Set(v8Strings::isDir.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_directory));
    js_data->Set(v8Strings::isSymLink.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_symlink));
    js_data->Set(v8Strings::lastModificationDate.Get(isolate),
                 gin::ConvertToV8(isolate, data.last_write_time));
    js_data->Set(v8Strings::readonly.Get(isolate),
                 gin::ConvertToV8(isolate, data.readonly));
    runner_->Call(function, v8::Undefined(isolate), js_data);
    js_data->Set(v8Strings::size.Get(isolate),
                 gin::ConvertToV8(isolate, data.file_size));
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusCallback);
};

void QueryFileStatus(const base::string16& filename,
                     v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const query_file_status_callback = make_scoped_refptr(
      new QueryFileStatusCallback(runner, callback));
  ScriptController::instance()->io_delegate()->QueryFileStatus(
      filename, base::Bind(&QueryFileStatusCallback::Run,
                           query_file_status_callback));
}

}  // namespace

namespace os {
namespace file {

v8::Handle<v8::ObjectTemplate> CreateObjectTemplate(v8::Isolate* isolate) {
  gin::ObjectTemplateBuilder builder(isolate);
  return builder
      .SetMethod("open_", &OpenFile)
      .SetMethod("stat_", &QueryFileStatus)
      .Build();
}

}  // namespace file
}  // namespace os
}  // namespace dom
