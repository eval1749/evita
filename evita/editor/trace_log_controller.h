// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_EDITOR_TRACE_LOG_CONTROLLER_H_
#define EVITA_EDITOR_TRACE_LOG_CONTROLLER_H_

#include <string>

#include "base/callback_forward.h"
#include "base/macros.h"

namespace editor {

class TraceLogController final {
 public:
  using OutputCallback =
      base::Callback<void(const std::string& json, bool has_more_events)>;

  TraceLogController();
  ~TraceLogController();

  void StartRecording(const std::string& config_string);
  void StopRecording(const OutputCallback& callback);

 private:
  DISALLOW_COPY_AND_ASSIGN(TraceLogController);
};

}  // namespace editor

#endif  // EVITA_EDITOR_TRACE_LOG_CONTROLLER_H_
