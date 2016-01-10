# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from optparse import OptionParser
import os
import sys

# SCRIPT_DIR = evita/dom/css
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
ROOT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, os.pardir, os.pardir,
                                         os.pardir))
CSS_DIR = os.path.normpath(os.path.join(ROOT_DIR, 'evita', 'visuals', 'css'))
THIRD_PARTY_DIR = os.path.normpath(os.path.join(ROOT_DIR, 'third_party'))
sys.path.append(THIRD_PARTY_DIR)
sys.path.append(CSS_DIR)

import jinja2
from css_properties import parse_css_model

TEMPLATES_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, 'templates'))

######################################################################
#
# Jinja2 Helper
#


def initialize_jinja_env(cache_dir):
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(TEMPLATES_DIR),
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
# Generators
#
class Generator(object):

    def __init__(self, options):
        self._jinja_env = initialize_jinja_env(options.cache_directory)
        self._output_dir = options.output_directory

    def generate(self, model):
        template_names = [
            'CSSStyleDeclaration.idl',
            'css_style_declaration.js',
            'css_value_simple_parsers.cc',
        ]
        context = self.make_context(model)
        for template_name in template_names:
            template = self._jinja_env.get_template(template_name)
            output_path = os.path.join(self._output_dir, template_name)
            with open(output_path, 'wt') as output:
                contents = template.render(context)
                output.write(contents)
            # Move to project to root to locate clang-format command
            # os.chdir(ROOT_DIR)
            # os.system('clang-format -i %s' % output_path)

    def make_context(self, model):
        return {
            'primitives': self.make_primitives(model),
            'properties': self.make_properties(model),
            'types': self.make_types(model),
        }

    def make_primitives(self, model):
        return model.primitive_types

    def make_properties(self, model):
        properties = []
        for css_property in model.properties:
            capitalized_name = css_property.name
            camel_case_name = capitalized_name[
                0].lower() + capitalized_name[1:]
            properties.append({
                'Name': capitalized_name,
                'Parameter': 'const %s&' % css_property.name,
                'camelCaseName': camel_case_name,
                'id': css_property.property_id,
                'name': css_property.underscore,
                'type': self.make_type(css_property.css_type),
                'text': '"%s"' % css_property.text,
            })
        return properties

    def make_type(self, css_type):
        return {
            'Name': css_type.name,
            'Parameter': css_type.to_parameter_type(),
            'Return': css_type.to_parameter_type(),
            'initial': css_type.initial_value,
            'is_compound': css_type.is_compound,
            'is_enum': css_type.is_enum,
            'is_keyword': css_type.is_keyword,
            'is_primitive': css_type.is_primitive,
            'members': [self.make_type(member) for member in css_type.members],
            'name': css_type.underscore,
            'text': '"%s"' % css_type.text,
        }

    def make_types(self, model):
        return [self.make_type(css_type) for css_type in model.types]


######################################################################
#
# Main
#
def parse_options():
    parser = OptionParser()
    parser.add_option('--cache-directory',
                      help='cache directory, defaults to output directory')
    parser.add_option('--output-directory')
    # ensure output comes last, so command line easy to parse via regexes
    parser.disable_interspersed_args()

    options, args = parser.parse_args()
    if options.output_directory is None:
        parser.error('Must specify output directory using --output-directory.')
    if len(args) != 1:
        parser.error(
            'Must specify exactly 1 input file as argument, but %d given.' %
            len(args))
    return options, os.path.realpath(args[0])


def main():
    options, input_file_name = parse_options()
    model = parse_css_model(open(input_file_name, 'rt').readlines())
    Generator(options).generate(model)


if __name__ == '__main__':
    sys.exit(main())
