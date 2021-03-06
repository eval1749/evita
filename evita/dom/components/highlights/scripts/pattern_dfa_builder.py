# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import deque
from pattern_nfa import NfaGraphVisitor, NfaGraphWalker

DEBUG = False

MAX_CHAR_CODE = 128


class Alphabets(object):
    """Represents alphabets of DFA"""

    def __init__(self, alphabets, alphabet_map, char_code_map):
        """
        - |alphabets| list of alphabet, e.g. [0, 1, 2, ...].
        - |alphabet_map| A mapping from alphabet to char code.
        - |char_code_map| A mapping from char code to alphabet.
        """
        self._alphabets = alphabets
        self._alphabet_map = alphabet_map
        assert len(char_code_map) == MAX_CHAR_CODE, len(char_code_map)
        assert max(char_code_map) == max(alphabets), \
            'alphabets=%s char_code_map=%s' % (alphabets, char_code_map)
        self._char_code_map = char_code_map

    @property
    def alphabet_map(self):
        return self._alphabet_map

    @property
    def char_code_map(self):
        return self._char_code_map

    def alphabet_of(self, char_code):
        return self.char_code_map[char_code]

    def char_code_of(self, alphabet):
        return self._alphabet_map[alphabet][0]

    def char_codes_of(self, alphabet):
        return self._alphabet_map[alphabet]

    def __iter__(self):
        return iter(self._alphabets)


class AlphabetsBuilder(NfaGraphVisitor):
    """Build minimum alphabet set from labels in NFA graph.
    This builder computes following alphaets from /[a][a-z0-9]+[^x]/:
        {1:[0-9b-wyz], 2:[a], 3:[x]}
    """

    def __init__(self):
        super(AlphabetsBuilder, self).__init__()
        self._classify_map = dict()

    def build(self, nfa_graph):
        self._classify(nfa_graph)
        return self._compute()

    def _classify(self, nfa_graph):
        self._classify_map.clear()
        walker = NfaGraphWalker(self)
        walker.visit(nfa_graph)

    def _compute(self):
        labels_map = dict()
        others = []
        alphabet_map = [others]
        char_code_map = [0] * MAX_CHAR_CODE
        for char_code in range(MAX_CHAR_CODE):
            if not(char_code in self._classify_map):
                others.append(char_code)
                continue
            labels = self._classify_map[char_code]
            key = ''.join(labels)
            if key in labels_map:
                (alpahbet, char_codes) = labels_map[key]
                char_code_map[char_code] = alpahbet
                char_codes.append(char_code)
                continue
            alphabet = len(alphabet_map)
            char_codes = [char_code]
            labels_map[key] = (alphabet, char_codes)
            alphabet_map.append(char_codes)
            char_code_map[char_code] = alphabet
        return Alphabets(range(len(alphabet_map)), alphabet_map, char_code_map)

    # NfaGraphVisitor method
    def process_range(self, label, min_code, max_code):
        assert min_code <= max_code, 'min=%d max=%d' % (min_code, max_code)
        for char_code in range(min_code, max_code + 1):
            if char_code in self._classify_map:
                self._classify_map[char_code].add(str(label))
                continue
            self._classify_map[char_code] = set(str(label))


class SimpleAlphabetsBuilder(NfaGraphVisitor):
    """Build small alphabet set from labels in NFA graph.
    This builder computes following alphaets from /[a][a-z0-9]+[^x]/:
        {1:[0-9], 2:[a], 3:[b-w], 4:[x], 5:[y-z]}

    An implementation is followed by [1].

    [1] https://github.com/google/re2/blob/master/re2/dfa.cc
    """

    def __init__(self):
        super(SimpleAlphabetsBuilder, self).__init__()
        self._start_set = set()
        self._used_chars = set()

    def build(self, nfa_graph):
        self._classify(nfa_graph)
        return self._compute()

    def _classify(self, nfa_graph):
        self._used_chars.clear()
        self._start_set.clear()
        walker = NfaGraphWalker(self)
        walker.visit(nfa_graph)

    def _compute(self):
        others = []
        alphabet_map = [others]
        char_code_map = [0] * MAX_CHAR_CODE
        char_codes = []
        for char_code in range(MAX_CHAR_CODE):
            if not(char_code in self._used_chars):
                others.append(char_code)
                continue
            if char_code in self._start_set:
                char_codes = [char_code]
                alphabet_map.append(char_codes)
            else:
                char_codes.append(char_code)
            char_code_map[char_code] = len(alphabet_map) - 1
        assert max(char_code_map) == len(alphabet_map) - 1, \
            'used=%s alphabets=%s char_code_map=%s starts=%s' % (
                self._used_chars, alphabet_map, char_code_map, self._start_set)
        return Alphabets(range(len(alphabet_map)), alphabet_map, char_code_map)

    # NfaGraphVisitor method
    def process_range(self, label, min_code, max_code):
        assert min_code <= max_code, 'min=%d max=%d' % (min_code, max_code)
        self._start_set.add(min_code)
        self._start_set.add(max_code + 1)
        self._used_chars.update(range(min_code, max_code + 1))


class DfaEdge(object):

    def __init__(self, alphabet, from_node, to_node):
        self._from_node = from_node
        self._label = set([alphabet])
        self._to_node = to_node

    @property
    def label(self):
        return self._label

    @property
    def from_node(self):
        return self._from_node

    @property
    def to_node(self):
        return self._to_node

    def add_alphabet(self, alphabet):
        self._label.add(alphabet)


class DfaGraph(object):
    """Represents DFA graph"""

    def __init__(self, alphabets):
        self._alphabets = alphabets
        self._nodes = []

    @property
    def alphabets(self):
        return self._alphabets

    @property
    def nodes(self):
        return self._nodes

    def finish(self, nodes):
        assert len(self._nodes) == 0
        assert len(nodes) > 0
        if DEBUG:
            print '\n', 'DfaGraph'
            for node in nodes:
                print '  ', str(node)
        self._nodes = nodes


class DfaNode(object):
    """Represents DFA state."""

    def __init__(self, graph, index, states):
        self._graph = graph
        self._index = index
        self._in_edges = []
        self._out_edges = []
        # |_states| holds a set of NFA states.
        self._states = states
        self._transitions = Transitions(self)

    @property
    def graph(self):
        return self._graph

    @property
    def index(self):
        return self._index

    @property
    def in_edges(self):
        return self._in_edges

    @property
    def is_acceptable(self):
        for state in self._states:
            if state.is_acceptable:
                return True
        return False

    @property
    def out_edges(self):
        return self._out_edges

    @property
    def transitions(self):
        """Returns char code to node set map."""
        return self._transitions

    @property
    def states(self):
        return self._states

    def add_transition(self, alphabet, to_node):
        self._transitions.add(alphabet, to_node)
        for present in self._out_edges:
            if present.to_node == to_node:
                present.add_alphabet(alphabet)
                return
        new_edge = DfaEdge(alphabet, self, to_node)
        self._out_edges.append(new_edge)
        to_node._in_edges.append(new_edge)

    def set_index(self, new_index):
        self._index = new_index

    def transit(self, char_code):
        return self._transitions.get(char_code)

    def __cmp__(self, other):
        if other == None:
            return False
        return cmp(self._index, other._index)

    def __eq__(self, other):
        if other == None:
            return False
        return self._index == other._index

    def __ne__(self, other):
        if other == None:
            return True
        return self._index != other._index

    def __str__(self):
        return 'DfaNode(%d, %s, nfa={%s} in={%s} out={%s} transitions=%s)' % (
            self._index,
            'A' if self.is_acceptable else '-',
            ', '.join(sorted(['%d:%s' % (state.index, state.group_name)
                              for state in self._states])),
            ', '.join(map(str,  sorted([edge.from_node.index
                                        for edge in self.in_edges]))),
            ', '.join(map(str,  sorted([edge.to_node.index
                                        for edge in self.out_edges]))),
            str(self._transitions))


class Partition(object):
    """Set of DfaNode"""

    def __init__(self, index, nodes):
        self._index = index
        self._nodes = list(nodes)

    @property
    def index(self):
        return self._index

    @property
    def members(self):
        assert len(self._nodes) >= 1
        return self._nodes

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
        self._nodes = list(nodes)

    def _sort_key(self):
        value = 0
        for node in self._nodes:
            for state in node.states:
                value = max(value, state.sort_key)
        return value

    def __cmp__(self, other):
        return cmp(self._sort_key(), other._sort_key())

    def __eq__(self, other):
        if other == None:
            return False
        return self._index == other._index

    def __str__(self):
        return 'Partition(%d, {%s})' % (
            self._index,
            ', '.join([str(node.index) for node in self._nodes]))


class Transitions(object):
    """A mapping of character code to set of DFA nodes."""

    def __init__(self, from_node):
        self._char_code_to_nodes = dict()
        self._from_node = from_node

    @property
    def mapping(self):
        return self._char_code_to_nodes

    def add(self, alphabet, node):
        char_codes = node.graph.alphabets.char_codes_of(alphabet)
        for char_code in char_codes:
            if char_code in self._char_code_to_nodes:
                self._char_code_to_nodes[char_code].add(node)
                continue
            self._char_code_to_nodes[char_code] = set([node])

    def get(self, char_code):
        if char_code in self._char_code_to_nodes:
            return self._char_code_to_nodes[char_code]
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
                result.append(vchr(min_code))
            else:
                result.append('%s-%s' % (vchr(min_code), vchr(max_code)))
            min_code = max_code = char_code
        if min_code != -1:
            if min_code == max_code:
                result.append(vchr(min_code))
            else:
                result.append('%s-%s' % (vchr(min_code), vchr(max_code)))
        return ''.join(result)

    def _complement_of(self, char_codes):
        assert len(char_codes) > 0
        result = []
        for char_code in range(MAX_CHAR_CODE):
            if char_code in char_codes:
                continue
            result.append(char_code)
        return result

    def _entry_to_string(self, node, char_codes):
        if len(char_codes) == MAX_CHAR_CODE:
            return '.->%d' % node.index
        if char_codes[0] == 0:
            return '[^%s]->%d' % (
                self._char_codes_to_str(self._complement_of(char_codes)),
                node.index)
        return '[%s]->%d' % (self._char_codes_to_str(char_codes), node.index)

    def __str__(self):
        node_to_alphabets = dict()
        for char_code in self._char_code_to_nodes.keys():
            for node in self._char_code_to_nodes[char_code]:
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
        pass

    def build(self, nfa_graph):
        """Build DFA from NFA |start_state|."""
        alpahbets = AlphabetsBuilder().build(nfa_graph)
        graph = DfaGraph(alpahbets)
        start_state = nfa_graph.start_node
        if DEBUG:
            print 'build', str(start_state)
        initial_node = DfaNode(graph, 0, closure_of(start_state))
        nodes = [initial_node]
        queue = deque(nodes)
        while len(queue) > 0:
            current = queue.popleft()
            for alphabet in alpahbets:
                char_code = alpahbets.char_code_of(alphabet)
                states = next_states_of(current.states, char_code)
                if len(states) == 0:
                    continue
                found = False
                for node in nodes:
                    if node.states != states:
                        continue
                    self._add_transition(current, alphabet, node)
                    found = True
                if found:
                    continue
                new_node = DfaNode(graph, len(nodes), states)
                self._add_transition(current, alphabet, new_node)
                nodes.append(new_node)
                queue.append(new_node)
        graph.finish(nodes)
        return DfaOptimizer().optimize(graph)

    def _add_transition(self, from_node, alphabet, to_node):
        from_node.add_transition(alphabet, to_node)


class DfaOptimizer(object):
    """Minimize DFA states in |nodes| by grouping by acceptable and transition.
    """

    def __init__(self):
        self._alphabets = None
        # Map a node to a partition where node in.
        self._partition_map = dict()
        self._partitions = []

    def optimize(self, graph):
        """The entry pointer of DFA optimizer."""
        self._alphabets = graph.alphabets
        nodes = graph.nodes
        assert len(nodes) > 0
        if DEBUG:
            print 'optimize %d nodes' % len(nodes)
        if len(nodes) == 1:
            # No needs to optimize
            return graph
        self._setup(nodes)
        if len(nodes) == len(self._partitions):
            # |nodes| are already minimum.
            return graph

        # Split partitions
        changed = True
        while changed:
            changed = False
            for partition in list(self._partitions):
                if self._split(partition):
                    changed = True

        # Make nodes from partitions
        result = []
        result_map = dict()
        for partition in sorted(self._partitions):
            states = set()
            for member in partition.members:
                states.update(member.states)
            node = DfaNode(graph, len(result), states)
            result.append(node)
            result_map[partition.index] = node

        self._verify_partitions(nodes)

        # Make edges
        for partition in self._partitions:
            member = partition.members[0]
            node = result_map[partition.index]
            for alphabet in self._alphabets:
                char_code = self._alphabets.char_code_of(alphabet)
                states = member.transit(char_code)
                for target in self._find_partitions(states):
                    node.add_transition(alphabet, result_map[target.index])
            if DEBUG:
                print 'result', partition, node

        # Make graph and return it
        new_graph = DfaGraph(self._alphabets)
        new_graph.finish(DfaNodeSorter().sort(result))
        return new_graph

    def _find_partitions(self, nodes):
        if len(nodes) == 0:
            return []
        result = []
        for partition in self._partitions:
            for node in nodes:
                if partition.contains(node):
                    result.append(partition)
                    break
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
            char_code = self._alphabets.char_code_of(alphabet)
            if not self._is_same_transition(node1, node2, char_code):
                return False
        return True

    def _is_same_transition(self, node1, node2, char_code):
        partitions1 = self._find_partitions(node1.transit(char_code))
        partitions2 = self._find_partitions(node2.transit(char_code))
        return partitions1 == partitions2

    def _new_partition(self, nodes):
        if len(nodes) == 0:
            return
        partition = Partition(len(self._partitions), nodes)
        self._partitions.append(partition)

    def _setup(self, nodes):
        """Create initial partiions, acceptable nodes, and others."""
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
        """Splits |partition1| if possible."""
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
        if DEBUG:
            print '  Split', partition1, 'to', \
                  [node.index for node in nodes1], \
                  [node.index for node in nodes2]
        assert len(nodes1) + len(nodes2) == partition1.count()
        if len(nodes2) == 0:
            return False
        partition1.reset(nodes1)
        self._new_partition(nodes2)
        return True

    def _verify_partitions(self, nodes):
        count = 0
        for partition in self._partitions:
            count = count + partition.count()
        assert len(nodes) == count, '%s nodes=[%s]' % (
            ' '.join([str(partition) for partition in self._partitions]),
            ' '.join([str(node) for node in nodes]))


class DfaNodeSorter(object):
    """Sorts nodes by preorder DFS"""

    def __init__(self):
        self._visited_nodes = set()
        self._nodes = []

    def sort(self, nodes):
        # Note: For ".*a", start node has in edges from self and "a".
        start_nodes = []
        for node in nodes:
            if len(node.in_edges) > 0:
                continue
            start_nodes.append(node)
        assert len(start_nodes) == 1, '\n'.join([str(node) for node in nodes])
        self._visit(start_nodes[0])
        if len(nodes) != len(self._nodes):
            for node in nodes:
                print node in self._visited_nodes, node
            assert len(nodes) == len(self._nodes), \
                'We should have all input nodes(%d) in output(%d)' % (
                len(nodes), len(self._nodes))
        # Renumber |DfaNode|
        index = 0
        for node in self._nodes:
            node.set_index(index)
            index = index + 1
        return self._nodes

    def _visit(self, node):
        if node in self._visited_nodes:
            return
        self._nodes.append(node)
        self._visited_nodes.add(node)
        for edge in sorted(node.out_edges, key=lambda edge: edge.to_node.index):
            self._visit(edge.to_node)


######################################################################
#
# Utility Functions
#
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


def vchr(char_code):
    if char_code == 0x22 or char_code == 0x27:
        return '\\%s' % chr(char_code)
    if char_code == 0x09:
        return '\\t'
    if char_code == 0x0A:
        return '\\n'
    if char_code == 0x0D:
        return '\\r'
    if char_code >= 0x20 and char_code < 0x7F:
        return chr(char_code)
    return '\\u%04X' % char_code
