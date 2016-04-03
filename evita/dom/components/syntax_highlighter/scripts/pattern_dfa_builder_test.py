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


def alphabets_to_string(alphabets):
    result = []
    for alphabet in alphabets:
        if alphabet == 0:
            continue
        result.append('%d:[%s]' % (
            alphabet, char_codes_to_string(alphabets.char_codes_of(alphabet))))
    return '{%s}' % ', '.join(result)


def build(*patterns):
    builder = NfaBuilder()
    for pattern in patterns:
        builder.build(pattern.name, PatternParser(pattern.source).parse())
    dfa_builder = DfaBuilder()
    graph = dfa_builder.build(builder.finalize())
    result = []
    for node in graph.nodes:
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
    return (' '.join(result), alphabets_to_string(graph.alphabets))


def char_codes_to_string(char_codes):
    result = []
    min_code = max_code = -1
    for char_code in sorted(char_codes):
        if min_code == -1:
            min_code = max_code = char_code
            continue
        if max_code + 1 == char_code:
            max_code = char_code
            continue
        if min_code == max_code:
            result.append(chr(min_code))
        else:
            result.append('%s-%s' % (chr(min_code), chr(max_code)))
        min_code = max_code = char_code
    if min_code != -1:
        if min_code == max_code:
            result.append(chr(min_code))
        else:
            result.append('%s-%s' % (chr(min_code), chr(max_code)))
    return ''.join(result)


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
        char_codes = edge_map[node]
        if len(char_codes) == 128:
            result.append('.->%d' % node.index)
            continue
        if 0 in char_codes:
            result.append('[^%s]->%d' % (
                char_codes_to_string(complement_of(char_codes)), node.index))
            continue
        result.append('[%s]->%d' % (char_codes_to_string(char_codes),
                                    node.index))
    return ', '.join(result)


class DfaBuilderTest(unittest.TestCase):

    def test_alphabet(self):
        # TODO(eval1749): We should merge alphabet[3] and alphabet[5]
        self.assertEqual(('0:foo:([a]->1) '
                          '1:foo:([0-9a-z]->2) '
                          '2:foo:([x]->2, [^0-9a-z]->3, [0-9a-wy-z]->4) '
                          '3:FOO '
                          '4:FOO:([x]->2, [^0-9a-z]->3, [0-9a-wy-z]->4)',
                          '{1:[0-9], 2:[a], 3:[b-w], 4:[x], 5:[y-z]}'),
                         build(NamedPattern('foo', 'a[a-z0-9]+[^x]')))

    def test_basic(self):
        self.assertEqual(('0:name:([a]->1) 1:NAME', '{1:[a]}'),
                         build(NamedPattern('name', 'a')))
        self.assertEqual(('0:name:([a]->1) 1:NAME:([a]->1)', '{1:[a]}'),
                         build(NamedPattern('name', 'a+')))
        self.assertEqual(('0:([a]->1, [5]->2) 1:NAME 2:NUMBER',
                          '{1:[5], 2:[a]}'),
                         build(NamedPattern('name', 'a'),
                               NamedPattern('number', '5')))
        self.assertEqual(('0:([a]->1, [9]->2) '
                          '1:NAME:([a]->1) '
                          '2:NUMBER:([9]->2)',
                          '{1:[9], 2:[a]}'),
                         build(NamedPattern('name', 'a+'),
                               NamedPattern('number', '9+')))
        self.assertEqual(('0:name:([0-9A-Z_a-z]->1) '
                          '1:NAME:([0-9A-Z_a-z]->1)',
                          '{1:[0-9], 2:[A-Z], 3:[_], 4:[a-z]}'),
                         build(NamedPattern('name', '\\w+')))

    def test_double_quote(self):
        # Double quote string with "/" escape
        self.assertEqual(('0:string:(["]->2) '
                          '1:string:(.->2) '
                          '2:string:([/]->1, [^"/]->2, ["]->3) '
                          '3:STRING',
                          '{1:["], 2:[/]}'),
                         build(NamedPattern('string', '"([^/"]|/.)*"')))

    def test_block_comment(self):
        self.assertEqual(('0:comment:([/]->1) '
                          '1:comment:([*]->2) '
                          '2:comment:([^*]->2, [*]->3) '
                          '3:comment:([^*/]->2, [*]->3, [/]->4) '
                          '4:COMMENT',
                          '{1:[*], 2:[/]}'),
                         build(NamedPattern('comment',
                                            '/[*][^*]*[*]+([^*/][^*]*[*]+)*/')))

    def test_block_comment_greedy(self):
        self.assertEqual(('0:comment:([/]->1) 1:comment:([*]->2) '
                          '2:comment:([^*]->2, [*]->3) '
                          '3:comment:([^*/]->2, [*]->3, [/]->4) '
                          '4:COMMENT:([^*]->2, [*]->3)',
                          '{1:[*], 2:[/]}'),
                         build(NamedPattern('comment', '/[*].*[*]/')))

    def test_block_comment_lazy(self):
        # TODO(eval1749)-> Can we convert lazy quantifier to DFA?
        self.assertEqual(('0:comment:([/]->1) 1:comment:([*]->2) '
                          '2:comment:([^*]->2, [*]->3) '
                          '3:comment:([^*/]->2, [*]->3, [/]->4) '
                          '4:COMMENT:([^*]->2, [*]->3)',
                          '{1:[*], 2:[/]}'),
                         build(NamedPattern('comment', '/[*].*?[*]/')))

    def test_repeat_any(self):
        self.assertEqual(('0:name:([^a]->0, [a]->1) 1:NAME:([^a]->0, [a]->1)',
                          '{1:[a]}'),
                         build(NamedPattern('name', '.*a')))


if __name__ == '__main__':
    unittest.main()
