// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function $define(parentModule, moduleName, moduleFunction) {
  function moduleOf(moduleName) {
    const present = Object.getOwnPropertyDescriptor(parentModule, moduleName);
    if (present)
      return present.value;
    const newModule = {};
    Object.defineProperty(parentModule, moduleName, {value: newModule});
    return newModule;
  }
  const toModule = moduleOf(moduleName);
  function $export(moduleObject) {
    Object.keys(moduleObject).forEach(name => {
      toModule[name] = moduleObject[name];
    });
  }
  moduleFunction.call(parentModule, $export);
}
