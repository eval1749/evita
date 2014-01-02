# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'target_defaults': {
    'include_dirs+' : [
      # TODO(yosi): We should not have |<(DEPTH)/evita| in |include_dirs|.
      '<(DEPTH)/evita',
    ], # include_dirs
  }, # target_defaults

  'targets': [
    {
      'target_name': 'evita_dom_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/common/common.gyp:common',
        'dom',
        'ui',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gmock/include/',
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalDependencies': [
            'gdi32.lib',
            'comdlg32.lib',
            'advapi32.lib',
            'shell32.lib',
            'ole32.lib',
            'oleaut32.lib',
            'uuid.lib',
          ], # AdditionalDependencies
        }, # VCLinkerTool
      }, # msvs_settings
      'sources': [
        'precomp.cpp',
        'dom/abstract_dom_test.cc',
        'dom/document_unittest.cc',
        'dom/editor_unittest.cc',
        'dom/editor_window_unittest.cc',
        'dom/mock_view_impl.cc',
        'dom/range_unittest.cc',
        'dom/window_unittest.cc',
      ],
    }, # evita_dom_test
    {
      'target_name': 'evita_text_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/evita/evita.gyp:text',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
      ], # dependencies
      'include_dirs+' : [
        #'<(DEPTH)/evita/',
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalDependencies': [
            'gdi32.lib',
            'comdlg32.lib',
            'advapi32.lib',
            'shell32.lib',
            'ole32.lib',
            'oleaut32.lib',
            'uuid.lib',
          ], # AdditionalDependencies
        }, # VCLinkerTool
      }, # msvs_settings
      'sources': [
        'precomp.cpp',
        'text/buffer_unittest.cc',
      ],
    }, # evita_text_test
  ] # targets
}
