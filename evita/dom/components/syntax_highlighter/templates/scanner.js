// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

{% macro emit_actions(actions, depth) %}
{% set INDENT = ' ' * depth %}
{%  for action in actions %}
{{ INDENT }}{{ action %}}
{% endfor %}
{% endmacro %}

goog.scope(function() {
const kStateName = [
{% for state in states %}
  '{{state.NAME}}',
{% endfor %}
];

/**
 * @implements {lexers.Scanner}
 */
class {{Name}}Scanner {
  constructor() {
  }

  /**
   * implements lexers.Scanner.stateNameOf
   * @param {number} state
   * @return {string}
   */
  stateNameOf(state) {
    return kStateName[state];
  }

  /**
   * implements lexers.Scanner.updateState
   * @param {number} state
   * @param {number} charCode
   * @return {number}
   */
  updateState(state, charCode) {
    switch (state) {
{% for state in states %}
      case {{state.number}}: // {{state.NAME}}
{%      for rule in state.rules %}
{%        if rule.condition == 'otherwise' %}
{%          emit_actions(rule.actions, 8) %}
{%        else %}
        if ({{rule.condition}}) {
{%          emit_actions(rule.actions, 10) %}
        }
{%        endif %}
{%      endfor %}
{% endof %}
    }
  }
}

lexers.{{Name}}Sanner = {{Name}}Scanner;
});
