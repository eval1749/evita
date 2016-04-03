# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of self source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from pattern_lexer import PatternLexer


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

    def __init__(self, label, from_state, to_state, is_lazy):
        self._from_state = from_state
        self._is_lazy = is_lazy
        self._label = label
        self._to_state = to_state

    @property
    def is_lazy(self):
        return self._is_lazy

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


class NfaBuilder(object):

    def __init__(self):
        self._states = []
        self._group_name = ''
        self._initial_states = []

    def build(self, group_name, node):
        self._group_name = group_name
        from_state = self._new_state(None)
        self._initial_states.append(from_state)
        self._process(from_state, None, node)

    def finalize(self):
        if len(self._initial_states) == 1:
            return NfaGraph(self._states, self._initial_states[0])
        initial_state = self._new_state(None)
        for state in self._initial_states:
            self._make_edge(None, initial_state, state)
        return NfaGraph(self._states, initial_state)

    def _make_edge(self, label, from_state, to_state, is_lazy=False):
        if to_state == None:
            to_state = self._new_state(None)
        edge = NfaEdge(label, from_state, to_state, is_lazy)
        from_state.add_out_edge(edge)
        to_state.add_in_edge(edge)
        return to_state

    def _new_state(self, node):
        state = NfaState(self._group_name, len(self._states) + 1, node)
        self._states.append(state)
        return state

    def _process(self, from_state, to_state, node, is_lazy=False):
        if node.is_primary:
            return self._make_edge(node.token, from_state, to_state, is_lazy)
        if node.is_or:
            return self._process_or(from_state, to_state, node, is_lazy)
        if node.is_repeat:
            return self._process_repeat(from_state, to_state, node, is_lazy)
        if node.is_sequence:
            return self._process_sequence(from_state, to_state, node, is_lazy)
        raise Exception('Bad node %s' % str(node))

    def _process_or(self, from_state, to_state, node, is_lazy):
        for member in node.members:
            to_state = self._process(from_state, to_state, member, is_lazy)
        return to_state

    def _process_repeat(self, from_state, to_state, node, is_lazy):
        if node.min_count == 0:
            if node.is_infinity:
                repeat_end = self._process(from_state, from_state,
                                           node.expression, node.is_lazy)
                return self._make_edge(None, repeat_end, to_state, is_lazy)
            for index in xrange(1, node.max_count):
                from_state = self._process(
                    from_state, None, node.expression,
                    node.is_lazy if index == 1 else False)
            return self._process(from_state, to_state, node.expression)
        # {n,m} where n > 0
        for index in xrange(0, node.min_count):  # pylint: disable=W0612
            from_state = self._process(from_state, None, node.expression)
        if node.is_infinity:
            repeat_end = self._process(from_state, from_state, node.expression)
            return self._make_edge(None, repeat_end, to_state)
        for index in xrange(1, node.max_count):
            from_state = self._process(from_state, None, node.expression,
                                       node.is_lazy if index == 1 else False)
        return self._process(from_state, to_state, node.expression)

    def _process_sequence(self, from_state, to_state, node, is_lazy):
        index = 0
        for member in node.members:
            index = index + 1
            if index == len(node.members):
                return self._process(from_state, to_state, member, is_lazy)
            from_state = self._process(from_state, None, member)
        assert(False)
