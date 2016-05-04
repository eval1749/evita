# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import sys

CURSOR_COMMAND = """
/**
 * @this {!TextWindow}
 * @param {number} count
 */
function %(name)s(count = 1) {
  this.selection.modify(Unit.%(unit)s, %(direction)scount, Alter.%(alter)s);
}
Editor.bindKey(TextWindow, '%(key_combination)s', %(name)s);
"""

ARROW_DOWN = 'ArrowDown'
ARROW_LEFT = 'ArrowLeft'
ARROW_RIGHT = 'ArrowRight'
ARROW_UP = 'ArrowUp'
PAGE_DOWN = 'PageDown'
PAGE_UP = 'PageUp'
CTRL = 'Ctrl'
SHIFT = 'Shift'

CURSOR_KEYS = [ARROW_DOWN, ARROW_LEFT,
               ARROW_RIGHT, ARROW_UP, PAGE_DOWN, PAGE_UP]
MODIFIERS_LIST = [[], [CTRL], [SHIFT], [CTRL, SHIFT]]


def alter_of(modifiers, key):
    assert key != '', key
    if SHIFT in modifiers:
        return 'EXTEND'
    return 'MOVE'


def direction_of(modifiers, key):
    assert len(modifiers) <= 2, modifiers
    if key == ARROW_LEFT or key == ARROW_UP or key == PAGE_UP:
        return '-'
    return ''


def command_name_of(modifiers, key):
    return ''.join([
        alter_of(modifiers, key).lower(),
        'Forward' if direction_of(modifiers, key) == '' else 'Backward',
        unit_of(modifiers, key).title().replace('_', ''),
    ])


def key_combination_of(modifiers, key):
    return '+'.join(modifiers + [key])


def unit_of(modifiers, key):
    is_ctrl = CTRL in modifiers
    if key == ARROW_DOWN or key == ARROW_UP:
        return 'BRACKET' if is_ctrl else 'WINDOW_LINE'
    if key == ARROW_LEFT or key == ARROW_RIGHT:
        return 'WORD' if is_ctrl else 'CHARACTER'
    if key == PAGE_DOWN or key == PAGE_UP:
        return 'WINDOW' if is_ctrl else 'SCREEN'
    raise Exception('Bad key combination: %s' % key)


def main():
    if len(sys.argv) != 2:
        raise Exception('Usage %s: output_name' %
                        os.path.basename(sys.argv[0]))

    output_file_name = sys.argv[1]

    results = ['goog.scope(function() {']
    for modifiers in MODIFIERS_LIST:
        for key in CURSOR_KEYS:
            context = {
                'alter': alter_of(modifiers, key),
                'direction': direction_of(modifiers, key),
                'key_combination': key_combination_of(modifiers, key),
                'name': command_name_of(modifiers, key),
                'unit': unit_of(modifiers, key),
            }
            result = CURSOR_COMMAND % context
            results.append(result)
    results.append('});\n')

    with open(output_file_name, 'wt') as output_file:
        output_file.write('\n'.join(results))

    return 0

if __name__ == '__main__':
    sys.exit(main())
