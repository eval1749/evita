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
    {
      'target_name': 'base_unittests',
      'type': 'executable',
      'include_dirs+': [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'dependencies': [
        'base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
      ], # dependencies
      'sources': [
        'castable_test.cc',
        'strings/string_piece_test.cc',
        'strings/utf_string_conversions_test.cc',
      ], # sources
    },
  ], # targets
}
