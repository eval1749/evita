# Copyright (c) 2013-2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'ui',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'msvs_precompiled_header': '<(DEPTH)/build/precompile.h',
      'msvs_precompiled_source': '<(DEPTH)/build/precompile.cc',
      'sources': [
        '<(DEPTH)/build/precompile.cc',

        'animation/animatable.cc',
        'animation/animatable.h',
        'animation/animatables.cc',
        'animation/animatables.h',
        'animation/animatable_window.cc',
        'animation/animatable_window.h',
        'animation/animation_observer.cc',
        'animation/animation_observer.h',
        'animation/animation_value.cc',
        'animation/animation_value.h',
        'animation/animator.cc',
        'animation/animator.h',
        'animation/window_animator.cc',
        'animation/window_animator.h',

        'base/ime/text_composition.cc',
        'base/ime/text_composition.h',
        'base/ime/text_input_client.h',
        'base/ime/text_input_client.cc',
        'base/ime/text_input_client_win.h',
        'base/ime/text_input_client_win.cc',
        'base/ime/text_input_delegate.h',
        'base/ime/text_input_delegate.cc',

        'base/selection_model.cc',
        'base/table_model.cc',
        'base/table_model_observer.cc',

        'events/event.cc',
        'events/event.h',
        'events/mouse_click_tracker.cc',
        'events/mouse_click_tracker.h',

        'compositor/canvas_for_layer.cc',
        'compositor/canvas_for_layer.h',
        'compositor/compositor.cc',
        'compositor/compositor.h',
        'compositor/layer.cc',
        'compositor/layer.h',
        'compositor/layer_animation.cc',
        'compositor/layer_animation.h',
        'compositor/layer_content.cc',
        'compositor/layer_content.h',
        'compositor/layer_owner.cc',
        'compositor/layer_owner.h',
        'compositor/root_layer.cc',
        'compositor/root_layer.h',

        'controls/button_control.cc',
        'controls/checkbox_control.cc',
        'controls/control.cc',
        'controls/control_controller.cc',
        'controls/label_control.cc',
        'controls/radio_button_control.cc',
        'controls/scroll_bar.cc',
        'controls/scroll_bar_observer.cc',
        'controls/status_bar.cc',
        'controls/table_control.cc',
        'controls/table_control_observer.cc',
        'controls/text_field_control.cc',

        'caret.cc',
        'root_widget.cc',
        'system_metrics.cc',
        'system_metrics_observer.cc',
        'tooltip.cc',
        'tooltip.h',
        'widget.cc',
        'widget.h',
      ], # sources
    },
  ], # targets
}
