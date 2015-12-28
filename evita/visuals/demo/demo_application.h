// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DEMO_DEMO_APPLICATION_H_
#define EVITA_VISUALS_DEMO_DEMO_APPLICATION_H_

#include "base/macros.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DemoApplication
//
class DemoApplication final {
 public:
  DemoApplication();
  ~DemoApplication();

  void Run();
  void Quit();

 private:
  DISALLOW_COPY_AND_ASSIGN(DemoApplication);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DEMO_DEMO_APPLICATION_H_
