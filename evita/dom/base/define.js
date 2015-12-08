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
  let isExportCalled = false;
  function $export(moduleObject) {
    const keys = Object.keys(moduleObject);
    if (keys.length === 0)
      throw 'No names to export';
    keys.forEach(name => {
      toModule[name] = moduleObject[name];
    });
    isExportCalled = true;
  }
  moduleFunction.call(parentModule, $export);
  if (isExportCalled)
    return;
  throw 'You should call $export() in $define()';
}
