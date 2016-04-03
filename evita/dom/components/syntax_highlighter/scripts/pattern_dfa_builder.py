# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import deque

DEBUG = False


class DfaNode(object):
    """Represents DFA state."""

    def __init__(self, index, states):
        self._index = index
        # |_states| holds a set of NFA states.
        self._states = states
        self._transitions = Transitions()

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
    def transitions(self):
        """Returns char code to node set map."""
        return self._transitions

    @property
    def states(self):
        return self._states

    def add_transition(self, char_code, node):
        self._transitions.add(char_code, node)

    def transit(self, char_code):
        return self._transitions.get(char_code)

    def __cmp__(self, other):
        return cmp(self._index, other._index)

    def __str__(self):
        return 'DfaNode(%d, %s, nfa={%s} transitions=%s)' % (
            self._index,
            'A' if self.is_acceptable else '-',
            ', '.join(sorted([str(state.index) for state in self._states])),
            str(self._transitions))


class Partition(object):
    """Set of DfaNode"""

    def __init__(self, index, nodes):
        self._index = index
        self._nodes = nodes

    @property
    def index(self):
        return self._index

    @property
    def member(self):
        assert len(self._nodes) >= 1
        return self._nodes[0]

    @property
    def nodes(self):
        return self._nodes

    def contains(self, node):
        for present in self._nodes:
            if present == node:
                return True
        return False

    def count(self):
        return len(self._nodes)

    def group_name(self):
        names = set()
        for node in self._nodes:
            for state in node.states:
                names.add(state.group_name)
        return ' '.join(names)

    def reset(self, nodes):
        self._nodes = nodes

    def _sort_key(self):
        value = 0
        for node in self._nodes:
            for state in node.states:
                value = max(value, state.sort_key)
        return value

    def __cmp__(self, other):
        return cmp(self._sort_key(), other._sort_key())

    def __eq__(self, other):
        return self._index == other._index

    def __str__(self):
        return 'Partition(%d, %s)' % (
            self._index,
            ', '.join([str(node) for node in self._nodes]))


class Transitions(object):
    """A mapping of character code to set of DFA nodes."""

    def __init__(self):
        self._alphabet_to_nodes = dict()

    @property
    def mapping(self):
        return self._alphabet_to_nodes

    def add(self, char_code, node):
        if char_code in self._alphabet_to_nodes:
            self._alphabet_to_nodes[char_code].add(node)
            return
        self._alphabet_to_nodes[char_code] = set([node])

    def get(self, char_code):
        if char_code in self._alphabet_to_nodes:
            return self._alphabet_to_nodes[char_code]
        return set()

    def _char_codes_to_str(self, char_codes):
        result = []
        min_code = max_code = -1
        for char_code in char_codes:
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

    def _complement_of(self, char_codes):
        assert len(char_codes) > 0
        result = []
        for char_code in range(0, 128):
            if char_code in char_codes:
                continue
            result.append(char_code)
        return result

    def _entry_to_string(self, node, char_codes):
        if len(char_codes) == 128:
            return '.->%d' % node.index
        if char_codes[0] == 0:
            return '[^%s]->%d' % (
                self._char_codes_to_str(self._complement_of(char_codes)),
                node.index)
        return '[%s]->%d' % (self._char_codes_to_str(char_codes), node.index)

    def __str__(self):
        node_to_alphabets = dict()
        for char_code in self._alphabet_to_nodes.keys():
            for node in self._alphabet_to_nodes[char_code]:
                if node in node_to_alphabets:
                    node_to_alphabets[node].append(char_code)
                    continue
                node_to_alphabets[node] = [char_code]
        result = []
        for node in sorted(node_to_alphabets.keys()):
            result.append(self._entry_to_string(node, node_to_alphabets[node]))
        return '{%s}' % ', '.join(result)


class DfaBuilder(object):
    """[1] has an explanation about RE-NFA to DFA.
    [1] https://swtch.com/~rsc/regexp/regexp1.html
    """

    def __init__(self):
        self._alphabets = range(0, 128)

    def build(self, nfa_graph):
        """Build DFA from NFA |start_state|."""
        start_state = nfa_graph.start_node
        if DEBUG:
            print 'build', str(start_state)
        initial_node = DfaNode(0, closure_of(start_state))
        nodes = [initial_node]
        queue = deque(nodes)
        while len(queue) > 0:
            current = queue.popleft()
            for char_code in self._alphabets:
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
                new_node = DfaNode(len(nodes), states)
                current.add_transition(char_code, new_node)
                nodes.append(new_node)
                queue.append(new_node)
        if DEBUG:
            for node in nodes:
                print '  ', str(node)
        return DfaOptimizer().optimize(nodes)


class DfaOptimizer(object):
    """Minimize DFA states in |nodes| by grouping by acceptable and transition.
    """

    def __init__(self):
        self._alphabets = range(0, 128)
        # Map a node to a partition where node in.
        self._partition_map = dict()
        self._partitions = []
        self._queue = deque()

    def _find_partitions(self, nodes):
        if len(nodes) == 0:
            return []
        result = []
        for partition in self._partitions:
            for node in nodes:
                if partition.contains(node):
                    result.append(partition)
        if len(result) == 0:
            partitions = []
            for partition in self._partitions:
                partitions.append(str(partition))
            raise Exception('No partition for {%s} in [%s]' % (
                ', '.join([str(node)for node in nodes]),
                ', '.join(partitions)))
        return result

    def _is_same_behavior(self, node1, node2):
        if node1.is_acceptable != node2.is_acceptable:
            return False
        group1 = set([state.group_name for state in node1.states])
        group2 = set([state.group_name for state in node2.states])
        if group1 != group2:
            return False
        for alphabet in self._alphabets:
            if not self._is_same_transition(node1, node2, alphabet):
                return False
        return True

    def _is_same_transition(self, node1, node2, alphabet):
        partitions1 = self._find_partitions(node1.transit(alphabet))
        partitions2 = self._find_partitions(node2.transit(alphabet))
        return partitions1 == partitions2

    def _new_partition(self, nodes):
        if len(nodes) == 0:
            return
        partition = Partition(len(self._partitions), nodes)
        self._partitions.append(partition)
        if len(nodes) == 1:
            return
        self._queue.append(partition)

    def optimize(self, nodes):
        assert len(nodes) > 0
        if DEBUG:
            print 'optimize', ' '.join([str(node) for node in nodes])
        if len(nodes) == 1:
            return nodes
        self._setup(nodes)
        if len(nodes) == len(self._partitions):
            return nodes
        assert len(self._queue) >= 1
        while len(self._queue) > 0:
            partition = self._queue.popleft()
            self._split(partition)
        result = []
        result_map = dict()
        for partition in sorted(self._partitions):
            original = partition.member
            node = DfaNode(len(result), original.states)
            result.append(node)
            result_map[partition] = node

        count = 0
        for partition in self._partitions:
            count = count + partition.count()
        assert len(nodes) == count, '%s nodes=[%s]' % (
            ' '.join([str(partition) for partition in self._partitions]),
            ' '.join([str(node) for node in nodes]))

        for partition in self._partitions:
            original = partition.member
            node = result_map[partition]
            for char_code in self._alphabets:
                states = original.transit(char_code)
                for target in self._find_partitions(states):
                    node.add_transition(char_code, result_map[target])
        if DEBUG:
            for node in result:
                print 'result', str(node)
        return result

    def _setup(self, nodes):
        accept_nodes = []
        not_accept_nodes = []
        for node in nodes:
            if node.is_acceptable:
                accept_nodes.append(node)
            else:
                not_accept_nodes.append(node)
        self._new_partition(accept_nodes)
        self._new_partition(not_accept_nodes)

    def _split(self, partition1):
        assert partition1.count() > 0
        if partition1.count() == 1:
            return
        nodes = partition1.nodes
        node = nodes[0]
        nodes1 = [node]
        nodes2 = []
        for other in nodes[1:]:
            if self._is_same_behavior(node, other):
                nodes1.append(other)
            else:
                nodes2.append(other)
        assert len(nodes1) + len(nodes2) == partition1.count()
        partition1.reset(nodes1)
        self._new_partition(nodes2)
        if len(nodes1) == 1 or len(nodes2) == 0:
            return
        self._queue.append(partition1)


def closure_of(from_state):
    """Returns set of states which can be reached without consuming input."""
    return closure_of_internal(from_state, set())


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
