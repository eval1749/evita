# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from pattern_dfa_builder import DfaBuilder
from pattern_nfa_builder import NfaBuilder
from pattern_parser import PatternParser


class NamedPattern(object):

    def __init__(self, name, source):
        self._name = name
        self._source = source

    @property
    def name(self):
        return self._name

    @property
    def source(self):
        return self._source


def build(*patterns):
    builder = NfaBuilder()
    for pattern in patterns:
        builder.build(pattern.name, PatternParser(pattern.source).parse())
    dfa_builder = DfaBuilder()
    nodes = dfa_builder.build(builder.finalize())
    result = []
    for node in nodes:
        node_name = group_name_of(node)
        if len(node.transitions.mapping) == 0:
            result.append('%d:%s' % (node.index, node_name.upper()))
            continue
        if node_name != '':
            node_name = ':' + node_name
        if node.is_acceptable:
            node_name = node_name.upper()
        result.append('%s%s:(%s)' %
                      (node.index, node_name,
                       transition_map_to_string(node.transitions.mapping)))
    return ' '.join(result)


def transition_map_to_string(transition_map):
    edge_map = dict()
    for char_code in transition_map.keys():
        for node in transition_map[char_code]:
            if node in edge_map:
                edge_map[node].append(char_code)
                continue
            edge_map[node] = [char_code]
    result = []
    for node in sorted(edge_map.keys()):
        char_set = edge_map[node]
        if len(char_set) == 128:
            result.append('.->%d' % node.index)
            continue
        complement = ''
        if 0 in char_set:
            complement = '^'
            ranges = char_set_to_string(complement_of(char_set))
        else:
            ranges = char_set_to_string(char_set)
        result.append('[%s%s]->%d' % (complement, ''.join(ranges), node.index))
    return ', '.join(result)


def char_set_to_string(char_set):
    ranges = []
    from_char_code = -1
    to_char_code = -1
    for char_code in sorted(char_set):
        if from_char_code == -1:
            from_char_code = char_code
            to_char_code = char_code
            continue
        if to_char_code + 1 == char_code:
            to_char_code = char_code
            continue
        if from_char_code == to_char_code:
            ranges.append(chr(from_char_code))
        else:
            ranges.append('%s-%s' % (chr(from_char_code),
                                     chr(to_char_code)))
        from_char_code = char_code
        to_char_code = char_code
    if from_char_code != -1:
        if from_char_code == to_char_code:
            ranges.append(chr(from_char_code))
        else:
            ranges.append('%s-%s' % (chr(from_char_code),
                                     chr(to_char_code)))
    return ranges


def complement_of(numbers):
    result = []
    for char_code in xrange(0, 128):
        if char_code in numbers:
            continue
        result.append(char_code)
    return result


def group_name_of(node):
    names = set()
    for state in node._states:
        names.add(state.group_name)
    if len(names) != 1:
        return ''
    return str(names.pop())


class DfaBuilderTest(unittest.TestCase):

    def test_basic(self):
        self.assertEqual('0:name:([a]->1) 1:NAME',
                         build(NamedPattern('name', 'a')))
        self.assertEqual('0:name:([a]->1) 1:NAME:([a]->1)',
                         build(NamedPattern('name', 'a+')))
        self.assertEqual('0:([a]->1, [5]->2) 1:NAME 2:NUMBER',
                         build(NamedPattern('name', 'a'),
                               NamedPattern('number', '5')))
        self.assertEqual('0:([a]->1, [9]->2) '
                         '1:NAME:([a]->1) '
                         '2:NUMBER:([9]->2)',
                         build(NamedPattern('name', 'a+'),
                               NamedPattern('number', '9+')))
        self.assertEqual('0:name:([0-9A-Z_a-z]->1) 1:NAME:([0-9A-Z_a-z]->1)',
                         build(NamedPattern('name', '\\w+')))

    def test_double_quote(self):
        # Double quote string with "/" escape
        self.assertEqual('0:string:(["]->2) '
                         '1:string:(.->2) '
                         '2:string:([/]->1, [^"/]->2, ["]->3) '
                         '3:STRING',
                         build(NamedPattern('string', '"([^/"]|/.)*"')))

    def test_block_comment(self):
        self.assertEqual('0:comment:([/]->1) '
                         '1:comment:([*]->2) '
                         '2:comment:([^*]->2, [*]->3) '
                         '3:comment:([^*/]->2, [*]->3, [/]->4) '
                         '4:COMMENT',
                         build(NamedPattern('comment',
                                            '/[*][^*]*[*]+([^*/][^*]*[*]+)*/')))

    def test_block_comment2(self):
        # TODO(eval1749)-> Can we convert lazy quantifier to DFA?
        self.assertEqual('0:comment:([/]->1) 1:comment:([*]->2) '
                         '2:comment:([^*]->2, [*]->3) '
                         '3:comment:([^*/]->2, [*]->3, [/]->4) '
                         '4:COMMENT:([^*]->2, [*]->3)',
                         build(NamedPattern('comment', '/[*].*?[*]/')))

    def test_repeat_any(self):
        self.assertEqual('0:name:([^a]->0, [a]->1) 1:NAME:([^a]->0, [a]->1)',
                         build(NamedPattern('name', '.*a')))


if __name__ == '__main__':
    unittest.main()
