# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

module_path = os.path.dirname(os.path.realpath(__file__))
third_party_dir = os.path.normpath(os.path.join(
    module_path, os.pardir, os.pardir, os.pardir, os.pardir, 'third_party'))
templates_dir = os.path.normpath(
    os.path.join(module_path, os.pardir, 'templates'))

from lexer import Lexer
import model
from model_builder import DocumentBuilder

sys.path.insert(1, third_party_dir)
# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2

COMMA = ord(',')
LPAREN = ord('(')
NUMBER_SIGN = ord('#')
NEWLINE = ord('\n')
RPAREN = ord(')')
SEMI_COLON = ord(';')


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
        self._document = document

    def build(self):
        return {'states': self.make_states()}

    def make_states(self):
        number = 0
        states = []
        for state in self._document.states:
            states.append({
                'NAME': capital_name_of(state.name),
                'number': number,
            })
            number = number + 1
        return states


def generate(output_path, document):
    context = ContextBuilder(document).build()

    jinja_env = initialize_jinja_env(None)
    template = jinja_env.get_template('lexer.js')
    with open(output_path, 'wt') as output:
        contents = template.render(context)
        output.write(contents)


def process_input(input_path):
    lexer = Lexer(input_path)
    builder = DocumentBuilder()
    state = 'state'
    while True:
        token = lexer.next_token()
        if token == None:
            break
        if state == 'state':
            builder.start_state(token)
            state = 'rules'
            continue

        if state == 'rules':
            if token.is_label:
                builder.end_state()
                builder.start_state(token)
                state = 'rules'
                continue
            builder.start_rule(token)
            state = 'actions'
            continue

        if state == 'actions':
            if token.is_operator_of(SEMI_COLON):
                builder.end_rule()
                state = 'rules'
                continue
            if token.is_action:
                builder.start_action(token)
                state = 'arguments'
                continue
            builder.error(token, 'Bad action')

        if state == 'arguments':
            if token.is_operator_of(RPAREN):
                builder.end_arguments()
                state = 'actions'
                continue
            builder.add_argument(token)
            state = 'arguments_argument'
            continue

        if state == 'arguments_argument':
            if token.is_operator_of(RPAREN):
                builder.end_action()
                state = 'actions'
                continue
            if token.is_operator_of(COMMA):
                state = 'arguments_comma'
                continue
            builder.error(token, 'Expect comma')

        if state == 'arguments_comma':
            if token.is_name or token.is_char:
                builder.add_argument(token)
                state = 'arguments'
                continue
            builder.error(token, 'Expect name or char')

        builder.error(token, 'Bad state %s' % state)
    builder.end_state()
    return builder.build()


def capital_name_of(name):
    return name[0].upper() + name[1:]


def main():
    if len(sys.argv) != 3:
        raise Exception('Usage %s output_path input_path' %
                        os.path.basename(sys.argv[0]))
    output_path = sys.argv[1]
    input_path = sys.argv[2]
    document = process_input(input_path)
    generate(output_path, document)

if __name__ == '__main__':
    sys.exit(main())
