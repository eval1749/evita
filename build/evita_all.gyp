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
        '../common/common.gyp:*',
        '../evita/evita.gyp:*',
        '../gin/gin.gyp:*',
        '../third_party/hunspell/hunspell.gyp:*',
        '../v8/tools/gyp/v8.gyp:*',
      ], # dependencies
    } # All
  ], # target
}
