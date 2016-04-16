# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class NfaGraph(object):
    """Represents a NFA graph"""

    def __init__(self, nodes, start_node):
        self._nodes = nodes
        self._start_node = start_node

    @property
    def nodes(self):
        return self._nodes

    @property
    def start_node(self):
        return self._start_node


class NfaEdge(object):

    def __init__(self, label, from_state, to_state):
        self._from_state = from_state
        self._label = label
        self._to_state = to_state

    @property
    def label(self):
        return self._label

    @property
    def from_state(self):
        return self._from_state

    @property
    def to_state(self):
        return self._to_state


class NfaState(object):

    def __init__(self, group_name, index, node):
        self._group_name = group_name
        self._in_edges = []
        self._index = index
        self._node = node
        self._out_edges = []

    def add_in_edge(self, edge):
        for present in self._in_edges:
            if present.from_state == edge.from_state and \
               present.label == edge.label:
                raise Exception('%s already has an edge from %s' % (
                    edge.from_state.index,
                    edge.to_state.index))
        self._in_edges.append(edge)

    def add_out_edge(self, edge):
        for present in self._out_edges:
            if present.to_state == edge.to_state and \
               present.label == edge.label:
                raise Exception('%s already has edge to %s' % (
                    edge.from_state.index,
                    edge.to_state.index))
        self._out_edges.append(edge)

    @property
    def index(self):
        return self._index

    @property
    def group_name(self):
        return self._group_name

    @property
    def in_edges(self):
        return self._in_edges

    @property
    def is_acceptable(self):
        return len(self._out_edges) == 0

    @property
    def node(self):
        return self._node

    @property
    def out_edges(self):
        return self._out_edges

    @property
    def sort_key(self):
        if self.is_acceptable:
            return self._index * 1000
        return self._index

    def is_terminal(self):
        for edge in self._out_edges:
            if edge.label != None:
                return False
        return True

    def __str__(self):
        return 'NfaState(%d, %s, from={%s}, to={%s})' % (
            self._index, self.node,
            ', '.join(['%s:%d' % (edge.label, edge.from_state.index)
                       for edge in self._in_edges]),
            ', '.join(['%s:%d' % (edge.label, edge.to_state.index)
                       for edge in self._out_edges]))


class NfaGraphVisitor(object):

    def __init__(self):
        pass

    def process_range(self, label, min_code, max_code):
        raise Exception(
            'You should implement |process_range(%s, %d, %d)|' % (
                label, min_code, max_code))


class NfaGraphWalker(object):

    def __init__(self, callback):
        self._callback = callback

    def visit(self, graph):
        for node in graph.nodes:
            for edge in node.out_edges:
                self.visit_label(edge.label)

    def visit_label(self, label):
        if label == None:
            return
        if label.is_any:
            return
        if label.is_char_set:
            for member in label.members:
                self._visit_for_charset(label, member)
            return
        if label.is_known_set:
            self._visit_for_charset(label, label.known_set)
            return
        if label.is_literal:
            self._visit_for_range(label, label.char_code, label.char_code)
            return
        raise Exception('Unknown label %s' % str(label))

    def _visit_for_charset(self, label, member):
        if member.is_range:
            self._visit_for_range(label, member.min_char_code,
                                  member.max_char_code)
            return
        if member.is_digit:
            return self._visit_for_range(label, ord('0'), ord('9'))
        if member.is_space:
            # \t \v \n \r
            self._visit_for_range(label, 0x09, 0x0D)
            self._visit_for_range(label, ord(' '), ord(' '))
            return
        if member.is_word:
            self._visit_for_range(label, ord('A'), ord('Z'))
            self._visit_for_range(label, ord('a'), ord('z'))
            self._visit_for_range(label, ord('0'), ord('9'))
            self._visit_for_range(label, ord('_'), ord('_'))
            return
        raise Exception('NYI charset %s' % str(member))

    def _visit_for_range(self, label, min_code, max_code):
        return self._callback.process_range(label, min_code, max_code)
