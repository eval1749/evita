// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_V8_GLUE_ISOLATE_HOLDER_H_
#define EVITA_V8_GLUE_ISOLATE_HOLDER_H_

#include <memory>

#include "evita/v8_glue/v8_glue.h"

#include "evita/v8_glue/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/public/isolate_holder.h"
END_V8_INCLUDE

namespace v8_glue {

class PerIsolateData;

class IsolateHolder final : public gin::IsolateHolder {
 public:
  IsolateHolder();
  ~IsolateHolder() = default;

 private:
  std::unique_ptr<PerIsolateData> isolate_data_;

  DISALLOW_COPY_AND_ASSIGN(IsolateHolder);
};

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_ISOLATE_HOLDER_H_
