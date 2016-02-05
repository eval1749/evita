// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_GINX_ISOLATE_HOLDER_H_
#define EVITA_GINX_ISOLATE_HOLDER_H_

#include <memory>

#include "evita/ginx/ginx.h"

#include "evita/ginx/per_isolate_data.h"
BEGIN_V8_INCLUDE
#include "gin/public/isolate_holder.h"
END_V8_INCLUDE

namespace ginx {

class PerIsolateData;

class IsolateHolder final : public gin::IsolateHolder {
 public:
  IsolateHolder();
  ~IsolateHolder() = default;

 private:
  std::unique_ptr<PerIsolateData> isolate_data_;

  DISALLOW_COPY_AND_ASSIGN(IsolateHolder);
};

}  // namespace ginx

#endif  // EVITA_GINX_ISOLATE_HOLDER_H_
