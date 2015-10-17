# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'evita_all_tests',
      'type': 'none',
      'dependencies': [
        'evita_dom_unittests',
        'evita_gfx_unittests',
        'evita_text_unittests',
        'evita_view_model_unittests',
      ], # dependencies
    }, # evita_all_tests
    {
      'target_name': 'evita_dom_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom',
        # TODO(eval1749): We should not have "editor" dependency on
        # "evita_dom_unittests".
        'editor',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gmock/include/',
        '<(DEPTH)/testing/gtest/include/',
        '<(SHARED_INTERMEDIATE_DIR)/',
      ], # include_dirs
      'msvs_disabled_warnings': [
        4365, 4625, 4626, 4628, 4826,
      ],
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
        'precomp.cc',
        'dom/abstract_dom_test.cc',
        'dom/console_unittest.cc',
        'dom/text/document_unittest.cc',
        'dom/editor_unittest.cc',
        'dom/windows/editor_window_unittest.cc',
        'dom/encodings/text_decoder_unittest.cc',
        'dom/encodings/text_encoder_unittest.cc',
        'dom/errors_unittest.cc',
        'dom/events/composition_event_unittest.cc',
        'dom/events/document_event_unittest.cc',
        'dom/events/event_target_unittest.cc',
        'dom/events/event_unittest.cc',
        'dom/events/focus_event_unittest.cc',
        'dom/events/form_event_unittest.cc',
        'dom/events/keyboard_event_unittest.cc',
        'dom/events/mouse_event_unittest.cc',
        'dom/events/wheel_event_unittest.cc',
        'dom/events/ui_event_unittest.cc',
        'dom/events/view_event_handler_impl_unittest.cc',
        'dom/events/window_event_unittest.cc',

        'dom/forms/button_control_unittest.cc',
        'dom/forms/checkbox_control_unittest.cc',
        'dom/forms/form_unittest.cc',
        'dom/forms/form_window_unittest.cc',
        'dom/forms/label_control_unittest.cc',
        'dom/forms/radio_button_control_unittest.cc',
        'dom/forms/text_field_control_unittest.cc',
        'dom/forms/text_field_selection_unittest.cc',

        'dom/file_path_unittest.cc',
        'dom/jslib/ordered_set_unittest.cc',
        'dom/lexers/lexers_unittest.cc',
        'dom/mock_io_delegate.cc',
        'dom/mock_view_impl.cc',
        'dom/text/mutation_observer_unittest.cc',
        'dom/os/file_unittest.cc',
        'dom/os/process_unittest.cc',
        'dom/text/modes/modes_unittest.cc',
        'dom/windows/point_unittest.cc',
        'dom/windows/rect_unittest.cc',
        'dom/polyfill_unittest.cc',
        'dom/text/range_unittest.cc',
        'dom/text/regexp_unittest.cc',
        'dom/css/style_unittest.cc',
        'dom/windows/table_selection_unittest.cc',
        'dom/text/text_position_unittest.cc',
        'dom/windows/text_selection_unittest.cc',
        'dom/windows/text_window_unittest.cc',
        'dom/unicode_unittest.cc',
        'dom/windows/window_unittest.cc',
      ],
    }, # evita_dom_test
    {
      'target_name': 'evita_gfx_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/evita/evita.gyp:gfx',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'msvs_disabled_warnings': [
        4625, 4626,
        # L2 C4826 Conversion from 'type1 ' to 'type_2' is sign-extended. This
        # may cause unexpected runtime behavior.
        4826,
      ],
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
        'precomp.cc',
        'gfx/rect_f_unittest.cc',
      ],
    }, # evita_gfx_test
    {
      'target_name': 'evita_spellchecker_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/evita/evita.gyp:spellchecker',
      ], # dependencies
      'include_dirs+' : [
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
        'precomp.cc',
        'spellchecker/hunspell_engine_unittest.cc',
      ],
    }, # evita_spellchecker_test
    {
      'target_name': 'evita_text_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/evita/evita.gyp:text',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
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
        'precomp.cc',
        'css/style_sheet_unittest.cc',
        'text/buffer_unittest.cc',
        'text/encodings/encodings_unittest.cc',
        'text/marker_set_unittest.cc',
        'text/range_unittest.cc',
        'text/undo_stack_unittest.cc',
      ],
    }, # evita_text_test
    {
      'target_name': 'evita_view_model_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/evita/evita.gyp:views',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
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
        'precomp.cc',
        'views/table_model_unittest.cc',
      ],
    }, # evita_text_test
    {
      'target_name': 'evita_glue_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/evita/evita.gyp:v8_glue',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'msvs_disabled_warnings': [
        4625, 4626, 4826,
      ],
      'sources': [
        'precomp.cc',
        'v8_glue/runner_unittest.cc',
      ],
    }, # evita_text_test
  ] # targets
}
