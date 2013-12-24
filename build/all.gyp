# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:*',
        '../evita/evita.gyp:*',
        '../gin/gin.gyp:*',
        '../v8/tools/gyp/v8.gyp:*',
      ], # dependencies
    } # ALl
  ], # target
}
