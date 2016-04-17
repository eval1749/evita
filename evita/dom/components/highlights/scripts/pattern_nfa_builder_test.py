# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from pattern_nfa_builder import NfaBuilder
from pattern_parser import PatternParser


def edges_to_string(edges):
    return '(%s)' % ', '.join([
        '%s%s%s' % (label_to_string(edge.label), ':', str(edge.to_state.index))
        for edge in edges])


def label_to_string(label):
    if label == None:
        return '_'
    return str(label)


def state_to_string(state):
    if len(state.out_edges) == 0:
        return str(state.index)
    return '%d->%s' % (state.index, edges_to_string(state.out_edges))


class Walker:

    def __init__(self):
        self._states = []
        self._visited = set()

    def finish(self):
        return ' '.join([state_to_string(state) for state in self._states])

    def visit(self, state):
        if state in self._visited:
            return
        self._visited.add(state)
        self._states.append(state)
        for edge in state.out_edges:
            self.visit(edge.to_state)


def build(source):
    builder = NfaBuilder()
    builder.build('sample', PatternParser(source).parse())
    graph = builder.finalize()
    walker = Walker()
    walker.visit(graph.start_node)
    return walker.finish()


class NfaBuilderTest(unittest.TestCase):

    def test_basic(self):
        self.assertEqual('1->("a":2) 2', build('a'))
        self.assertEqual('1->("a":2) 2->("b":3) 3', build('ab'))
        self.assertEqual('1->("a":1, _:2) 2', build('a*'))
        self.assertEqual('1->("a":2) 2->(_:3, "a":3) 3', build('aa?'))
        self.assertEqual('1->("a":2) 2->("a":2, _:3) 3', build('a+'))
        self.assertEqual('1->("a":1, _:2) 2', build('a*?'))
        self.assertEqual('1->("a":1, _:2) 2->("b":3) 3', build('a*b'))
        self.assertEqual('1->("a":2, "b":2) 2', build('a|b'))

    def test_block_comment(self):
        self.assertEqual('1->("/":2) 2->("*":3) 3->([^*]:3, _:4) 4->("*":5) '
                         '5->("*":5, _:6) 6->([^/*]:7, _:10) 7->([^*]:7, _:8) '
                         '8->("*":9) 9->("*":9, _:6) 10->("/":11) 11',
                         build('/[*][^*]*[*]+([^/*][^*]*[*]+)*/'))

    def test_block_comment2(self):
        self.assertEqual('1->("/":2) 2->("*":3) 3->(.:3, _:4) 4->("*":5) '
                         '5->("/":6) 6',
                         build('/[*].*?[*]/'))

    def test_double_quote(self):
        # Double-quote string with "/" escape
        self.assertEqual('1->("\\"":2) 2->([^/\\"]:2, "/":3, _:4) 3->(.:2) '
                         '4->("\\"":5) 5',
                         build('"([^/"]|/.)*"'))

    def test_html_comment(self):
        self.assertEqual('1->("<":2) 2->("!":3) 3->("-":4) 4->("-":5) '
                         '5->(.:5, _:6) 6->("-":7) 7->("-":8) 8->(">":9) 9',
                         build('<!--.*-->'))

    def test_quantifiers(self):
        self.assertEqual('1->("/":2) 2->("*":3) 3->(.:3, _:4) 4->("*":5) '
                         '5->("/":6) 6',
                         build('/[*].*[*]/'))

    def test_repeat_any(self):
        self.assertEqual('1->(.:1, _:2) 2->("a":3) 3', build('.*a'))
        self.assertEqual('1->(.:1, _:2) 2->("a":3) 3->("b":4) 4',
                         build('.*ab'))


if __name__ == '__main__':
    unittest.main()
