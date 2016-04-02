# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import deque

DEBUG = False


class DfaState(object):
    """Represents DFA state."""

    def __init__(self, index, states):
        self._index = index
        # |_states| holds a set of NFA states.
        self._states = states
        self._transition_map = dict()
        if not DEBUG:
            return
        print self

    @property
    def index(self):
        return self._index

    @property
    def is_acceptable(self):
        for state in self._states:
            if state.is_acceptable:
                return True
        return False

    @property
    def transition_map(self):
        """Returns char code to node set map."""
        return self._transition_map

    @property
    def states(self):
        return self._states

    def add_transition(self, char_code, node):
        if char_code in self._transition_map:
            self._transition_map[char_code].add(node)
            return
        self._transition_map[char_code] = set([node])

    def __cmp__(self, other):
        return cmp(self._index, other._index)

    def __str__(self):
        return 'DfaState(%d, {%s})' % (
            self._index,
            ', '.join(sorted([str(state.name) for state in self._states])))


class DfaBuilder(object):
    """[1] has an explanation about RE-NFA to DFA.
    [1] https://swtch.com/~rsc/regexp/regexp1.html
    """

    def __init__(self):
        pass

    def build(self, start_state):
        """Build DFA from NFA |start_state|."""
        initial_node = DfaState(0, closure_of(start_state))
        nodes = [initial_node]
        queue = deque(nodes)
        while len(queue) > 0:
            current = queue.popleft()
            for char_code in xrange(0, 128):
                states = next_states_of(current.states, char_code)
                if len(states) == 0:
                    continue
                found = False
                for node in nodes:
                    if node.states != states:
                        continue
                    current.add_transition(char_code, node)
                    found = True
                if found:
                    continue
                new_node = DfaState(len(nodes), states)
                current.add_transition(char_code, new_node)
                nodes.append(new_node)
                queue.append(new_node)
        return optimize_dfa(nodes)


def closure_of(from_state):
    """Returns set of states which can be reached without consuming input."""
    return closure_of_internal(from_state, set())


def compute_key(node):
    result = sorted(set([state.group_name for state in node.states]))
    if node.is_acceptable:
        result.append('A')
    for char_code in range(0, 128):
        if not(char_code in node.transition_map):
            continue
        nodes = node.transition_map[char_code]
        result.append('%d:%s' % (
            char_code,
            ' '.join(sorted([str(next_node.index) for next_node in nodes]))))
    return ' '.join(result)


def closure_of_internal(from_state, result):
    if from_state in result:
        return result
    result.add(from_state)
    for edge in from_state.out_edges:
        if edge.label != None:
            continue
        closure_of_internal(edge.to_state, result)
    return result


def match(label, char_code):
    if label == None:
        return False
    if label.is_any:
        return True
    if label.is_boundary:
        raise Exception('NYI boundary match')
    if label.is_char_set:
        for member in label.members:
            if member.contains(char_code):
                return not label.is_complement
        return label.is_complement
    if label.is_literal:
        return label.char_code == char_code
    if label.is_known_set:
        return label.known_set.contains(char_code)
    raise Exception('Unsupported label %s' % str(label))


def next_states_of(from_states, char_code):
    """Returns set of NFA states which can be reached without consuming
    |char_code|."""
    result = set()
    for from_state in from_states:
        for edge in from_state.out_edges:
            if not match(edge.label, char_code):
                continue
            for state in closure_of(edge.to_state):
                result.add(state)
    return result


def optimize_dfa(nodes):
    """Minimize DFA states in |nodes| by grouping by acceptable and transition.
    """
    node_map = dict()
    redundant_map = dict()
    result = []
    for node in nodes:
        key = compute_key(node)
        if key in node_map:
            redundant_map[node] = node_map[key]
            continue
        node_map[key] = node
        result.append(node)
    if len(redundant_map) == 0:
        assert len(result) == len(nodes)
        return result
    for node in result:
        for char_code in node.transition_map:
            next_nodes = node.transition_map[char_code]
            for redundant in redundant_map.keys():
                if not(redundant in next_nodes):
                    continue
                next_nodes.remove(redundant)
                next_nodes.add(redundant_map[redundant])
    return optimize_dfa(result)
