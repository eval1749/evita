# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'targets': [
    {
      'target_name': 'regex_lib',
      'type': 'static_library',

      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],

      'sources': [
        'precomp.h',
        'regex.cc',
        'regex_compile.cc',
        'regex_debug.cc',
        'regex_exec.cc',
        'regex_node.cc',
        'regex_parse.cc',
        'regex_unicode.cc',
        'regex_util.cc',
      ], # sources
    }, # regex

    {
      'target_name': 'regex_test',
      'type': 'executable',
      'include_dirs+': [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'dependencies': [
        'regex_lib',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
      ], # dependencies
      'sources': [
        'regex_test.cc',
      ], # sources
    }, # regex_test
  ], # targets
}
