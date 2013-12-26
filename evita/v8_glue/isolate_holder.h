// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_isolate_holder_h)
#define INCLUDE_evita_v8_glue_isolate_holder_h

#include <memory>

#include "evita/v8_glue/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/public/isolate_holder.h"
END_V8_INCLUDE

namespace v8_glue {

class PerIsolateData;

class IsolateHolder : public gin::IsolateHolder {
  private: std::unique_ptr<PerIsolateData> isolate_data_;

  public: IsolateHolder();
  public: ~IsolateHolder() = default;

  DISALLOW_COPY_AND_ASSIGN(IsolateHolder);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_isolate_holder_h)
