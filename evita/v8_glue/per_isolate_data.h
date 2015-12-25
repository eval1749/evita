// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_V8_GLUE_PER_ISOLATE_DATA_H_
#define EVITA_V8_GLUE_PER_ISOLATE_DATA_H_

#include "base/macros.h"
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

class PerIsolateData final {
 public:
  explicit PerIsolateData(v8::Isolate* isolate);
  ~PerIsolateData();

  ConstructorMode construct_mode() const { return construct_mode_; }

  bool is_creating_wrapper() const {
    return construct_mode_ == kWrapExistingObject;
  }

  void set_construct_mode(ConstructorMode construct_mode) {
    construct_mode_ = construct_mode;
  }

  Runner* current_runner() const { return current_runner_; }
  void set_current_runner(Runner* runner) { current_runner_ = runner; }

  base::TaskRunner* task_runner() { return task_runner_.get(); }

  static PerIsolateData* From(v8::Isolate* isolate);

 private:
  ConstructorMode construct_mode_;
  Runner* current_runner_;
  v8::Isolate* isolate_;
  scoped_refptr<base::TaskRunner> task_runner_;

  DISALLOW_COPY_AND_ASSIGN(PerIsolateData);
};

class ConstructorModeScope final {
 public:
  ConstructorModeScope(v8::Isolate* isolate, ConstructorMode construct_mode)
      : construct_mode_(PerIsolateData::From(isolate)->construct_mode()),
        isolate_(isolate) {
    PerIsolateData::From(isolate_)->set_construct_mode(construct_mode);
  }

  ~ConstructorModeScope() {
    PerIsolateData::From(isolate_)->set_construct_mode(construct_mode_);
  }

 private:
  ConstructorMode const construct_mode_;
  v8::Isolate* isolate_;

  DISALLOW_COPY_AND_ASSIGN(ConstructorModeScope);
};

}  // namespace v8_glue

#endif  // EVITA_V8_GLUE_PER_ISOLATE_DATA_H_
