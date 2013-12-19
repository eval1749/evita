# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'evita_code': 1,
  }, # variables
  'targets': [
    {
      'target_name': 'evita_console',
      'type': 'executable',
      'sources': [
        'evita_console.cc',
      ], # sources
      'dependencies': [
        '<(DEPTH)/third_party/v8/tools/gyp/v8.gyp:v8',
      ], # dependencies
    },
  ], # targets
}
