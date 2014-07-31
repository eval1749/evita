# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'includes': [
    '../build/win_precompile.gypi',
  ], # includes

  'targets': [
    {
      'target_name': 'common',
      'type': '<(component)',
      'defines': [ 'COMMON_IMPLEMENTATION' ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ], # dependencies
      'sources': [
        'strings/atomic_string.cc',
        'strings/atomic_string.h',
        'timer/timer.cc',
        'win/native_window.cc',
        'win/point.cc',
        'win/point.h',
        'win/rect.cc',
        'win/registry.cc',
        'win/rect_ostream.cc',
        'win/singleton_hwnd.cc',
        'win/singleton_hwnd.h',
        'win/size.cc',
        'win/size.h',
      ], # sources
    }, # common

    {
      'target_name': 'common_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/testing/gtest.gyp:gtest',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'sources': [
        'castable_unittest.cc',
        'memory/ref_counted_unittest.cc',
        'memory/scoped_refptr_unittest.cc',
        'strings/atomic_string_unittest.cc',
        'tree/node_unittest.cc',
        'win/rect_unittest.cc',
      ], # sources
      'msvs_disabled_warnings': [
        4625, 4626, 4826,
      ],
    }, # common_test
  ], # targets

  'target_defaults': {
    'msvs_disabled_warnings': [
      # C4127: conditional expression is constant
      4127, # for DCHECK_XX
      # C4310: cast truncates constant value
      4310, # for base/basictypes.h
     ],
  }, # target_defaults
}
