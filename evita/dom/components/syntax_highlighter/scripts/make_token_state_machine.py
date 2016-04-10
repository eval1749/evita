# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
from xml.dom.minidom import parse, parseString

module_path = os.path.dirname(os.path.realpath(__file__))
third_party_dir = os.path.normpath(os.path.join(
    module_path, os.pardir, os.pardir, os.pardir, os.pardir, os.pardir,
    'third_party'))
print 'third_party_dir', third_party_dir
templates_dir = os.path.normpath(
    os.path.join(module_path, os.pardir, 'templates'))

sys.path.insert(1, third_party_dir)
# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2

from pattern_dfa_builder import DfaBuilder
from pattern_nfa_builder import NfaBuilder
from pattern_parser import PatternParser

ESCAPE_CHARS = {
    0x09: 't',
    0x0A: 'n',
    0x0B: 'v',
    0x0C: 'f',
    0x0D: 'r',
    0x27: '\'',
    0x5C: '\\',
}

######################################################################
#
# Jinja2 Helper
#


def initialize_jinja_env(cache_dir):
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        # Bytecode cache is not concurrency-safe unless pre-cached:
        # if pre-cached this is read-only, but writing creates a race
        # condition.
        bytecode_cache=jinja2.FileSystemBytecodeCache(cache_dir),
        keep_trailing_newline=True,  # newline-terminate generated files
        lstrip_blocks=True,  # so can indent control flow tags
        trim_blocks=True)
    return jinja_env


######################################################################
#
# ContextBuilder
#
class ContextBuilder(object):

    def __init__(self, document):
        self._context = dict()
        self._document = document

    def build(self):
        tokens = self._document.documentElement
        self._context['name'] = tokens.getAttribute('name')
        self._context['Name'] = capitalize(self._context['name'])
        self._process_tokens(tokens)
        return self._context

    def _process_tokens(self, tokens):
        nfa_builder = NfaBuilder()
        token_name_to_type_map = dict()
        token_name_to_type_map[''] = ''
        for child in tokens.childNodes:
            if child.nodeName != 'token':
                continue
            pattern = PatternParser(child.getAttribute('pattern')).parse()
            token_name = child.getAttribute('name')
            token_type = child.getAttribute('type')
            nfa_builder.build(token_name, pattern)
            token_name_to_type_map[token_name] = token_type
        dfa_builder = DfaBuilder()
        graph = dfa_builder.build(nfa_builder.finalize())

        alphabets = graph.alphabets
        nodes = graph.nodes

        # Alphabets
        self._context['alphabet_map'] = [
            char_codes_to_string(alphabets.char_codes_of(alphabet))
            for alphabet in alphabets
        ]
        max_alphabet = len(alphabets.alphabet_map) - 1
        self._context['alphabet_type'] = element_type_for(max_alphabet)
        self._context['max_alphabet'] = max_alphabet

        # States
        self._context['char_code_to_alphabet_map'] = \
            alphabets.char_code_map
        self._context['states'] = [
            {
                'comment': state_names_of(node),
                'is_acceptable': node.is_acceptable,
                'is_from_acceptable': is_from_acceptable(node),
                'index': node.index,
                'token_type': token_type_of(token_name_to_type_map, node),
                'transitions': compute_transitions(node),
            }
            for node in nodes
        ]
        max_state = max([node.index for node in nodes])
        self._context['max_state'] = max_state
        self._context['state_type'] = element_type_for(max_state)


def generate(output_path, document):
    context = ContextBuilder(document).build()

    jinja_env = initialize_jinja_env(None)
    template = jinja_env.get_template('token_state_machine.js')
    with open(output_path, 'wt') as output:
        contents = template.render(context)
        output.write(contents)


######################################################################
#
# Utility Functions
#
def capitalize(name):
    return name[0].upper() + name[1:]


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


def compute_transitions(node):
    """Returns mapping of alphabet to next state for |node| as an array."""
    result = []
    alphabets = node.graph.alphabets
    for alphabet in alphabets:
        char_code = alphabets.char_code_of(alphabet)
        targets = node.transit(char_code)
        if len(targets) == 0:
            result.append(0)
            continue
        if len(targets) == 1:
            result.append(next(iter(targets)).index)
            continue
        raise Exception('Invalid transition for alphabet %d in %s' % (
            alphabet, node))
    return result


def element_type_for(value):
    if value < 256:
        return 'Uint8'
    if value < 65536:
        return 'Uint16'
    return 'Uint32'


def is_from_acceptable(node):
    """Returns true if |node| has an edge from acceptable node."""
    for in_edge in node.in_edges:
        if in_edge.from_node.is_acceptable:
            return True
    return False


def state_names_of(node):
    state_names = set()
    for state in node.states:
        state_names.add(state.group_name)
    return '|'.join(state_names)


def token_type_of(type_map, node):
    acceptable_states = []
    types = set()
    for state in node.states:
        if state.is_acceptable:
            acceptable_states.append(state)
        types.add(type_map[state.group_name])
    if len(acceptable_states) == 1:
        return type_map[acceptable_states[0].group_name]
    assert len(acceptable_states) == 0, \
        'DFA node can not have multiple acceptable states'
    if len(types) != 1:
        return ''
    return next(iter(types))


def vchr(char_code):
    if char_code in ESCAPE_CHARS:
        return '\\%s' % ESCAPE_CHARS[char_code]
    if char_code < 0x20 or char_code >= 0x7F:
        return '\\u%04X' % char_code
    return chr(char_code)


######################################################################
#
# Entry Point
#
def main():
    if len(sys.argv) != 3:
        raise Exception('Usage %s output_path input_path' %
                        os.path.basename(sys.argv[0]))
    output_path = sys.argv[1]
    input_path = sys.argv[2]
    document = parse(input_path)
    generate(output_path, document)

if __name__ == '__main__':
    sys.exit(main())
