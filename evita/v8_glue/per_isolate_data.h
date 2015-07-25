// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_per_isolate_data_h)
#define INCLUDE_evita_v8_glue_per_isolate_data_h

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "evita/v8_glue/v8_glue.h"
BEGIN_V8_INCLUDE
#include "gin/per_isolate_data.h"
END_V8_INCLUDE

namespace base {
class TaskRunner;
}

namespace v8 {
class Isolate;
}

namespace v8_glue {

enum ConstructorMode {
  kWrapExistingObject,
  kCreateNewObject,
};

class Runner;

class PerIsolateData {
  private: ConstructorMode construct_mode_;
  private: Runner* current_runner_;
  private: v8::Isolate* isolate_;
  private: scoped_refptr<base::TaskRunner> task_runner_;

  public: PerIsolateData(v8::Isolate* isolate);
  public: ~PerIsolateData();

  public: ConstructorMode construct_mode() const { return construct_mode_; }

  public: bool is_creating_wrapper() const {
    return construct_mode_ == kWrapExistingObject;
  }

  public: void set_construct_mode(ConstructorMode construct_mode) {
    construct_mode_ = construct_mode;
  }

  public: Runner* current_runner() const { return current_runner_; }
  public: void set_current_runner(Runner* runner) { current_runner_ = runner; }

  public: base::TaskRunner* task_runner() {
    return task_runner_.get();
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
