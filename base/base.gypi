# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'target_defaults': {
    'variables': {
      'base_target': 0, # base.gyp:base sets base_target=1
    }, # variables
    'target_conditions': [
      ['base_target==1', {
        'sources': [
          'logging.cc',
        ], # sources
      }], # base_target==1
    ],
  }, # target_defaults
}
