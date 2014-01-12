// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_per_isolate_data_h)
#define INCLUDE_evita_v8_glue_per_isolate_data_h

#include "base/basictypes.h"
#include "evita/v8_glue/v8_glue.h"
BEGIN_V8_INCLUDE
#include "gin/per_isolate_data.h"
END_V8_INCLUDE

namespace v8 {
class Isolate;
}

namespace v8_glue {

enum ConstructorMode {
  kWrapExistingObject,
  kCreateNewObject,
};

class PerIsolateData {
  private: ConstructorMode construct_mode_;
  private: v8::Isolate* isolate_;

  public: PerIsolateData(v8::Isolate* isolate);
  public: ~PerIsolateData();

  public: ConstructorMode construct_mode() const { return construct_mode_; }

  public: bool is_creating_wrapper() const {
    return construct_mode_ == kWrapExistingObject;
  }

  public: void set_construct_mode(ConstructorMode construct_mode) {
    construct_mode_ = construct_mode;
  }

  public: static PerIsolateData* From(v8::Isolate* isolate);

  DISALLOW_COPY_AND_ASSIGN(PerIsolateData);
};

class ConstructorModeScope {
  private: ConstructorMode const construct_mode_;
  private: v8::Isolate* isolate_;

  public: ConstructorModeScope(v8::Isolate* isolate,
                             ConstructorMode construct_mode)
      : construct_mode_(PerIsolateData::From(isolate)->construct_mode()),
        isolate_(isolate) {
    PerIsolateData::From(isolate_)->set_construct_mode(construct_mode);
  }

  public: ~ConstructorModeScope() {
    PerIsolateData::From(isolate_)->set_construct_mode(construct_mode_);
  }

  DISALLOW_COPY_AND_ASSIGN(ConstructorModeScope);
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_per_isolate_data_h)
