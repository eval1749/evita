# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'includes': [
    '../build/win_precompile.gypi',
    'base.gypi',
  ], # includes

  'targets': [
    {
      'target_name': 'base',
      'type': '<(component)',
      'defines': [ 'BASE_IMPLEMENTATION' ],
      'variables': {
        'base_target': 1,
      }, # variables
    }, # base
  ], # targets
}
