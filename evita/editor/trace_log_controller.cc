// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/trace_log_controller.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/memory/ref_counted_memory.h"
#include "base/trace_event/trace_config.h"
#include "base/trace_event/trace_log.h"

namespace editor {

namespace {
class Wrapper final {
 public:
  explicit Wrapper(const TraceLogController::OutputCallback& callback)
      : callback_(callback) {}
  ~Wrapper() = default;

  void DidGetEvent(const scoped_refptr<base::RefCountedString>& string,
                   bool has_more_events);

 private:
  TraceLogController::OutputCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(Wrapper);
};
void Wrapper::DidGetEvent(const scoped_refptr<base::RefCountedString>& string,
                          bool has_more_events) {
  callback_.Run(string->data(), has_more_events);
  if (has_more_events)
    return;
  delete this;
}
}  // namespace

TraceLogController::TraceLogController() {}

TraceLogController::~TraceLogController() {}

void TraceLogController::StartRecording(const std::string& config_string) {
  base::trace_event::TraceConfig trace_config(config_string);
  base::trace_event::TraceLog::GetInstance()->SetEnabled(
      trace_config, base::trace_event::TraceLog::RECORDING_MODE);
}

void TraceLogController::StopRecording(const OutputCallback& callback) {
  base::trace_event::TraceLog::GetInstance()->SetDisabled();
  auto const wrapper = new Wrapper(callback);
  base::trace_event::TraceLog::GetInstance()->Flush(
      base::Bind(&Wrapper::DidGetEvent, base::Unretained(wrapper)));
}

}  // namespace editor
