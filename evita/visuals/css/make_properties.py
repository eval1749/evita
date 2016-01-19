# Copyright (c) 2015 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from optparse import OptionParser
import os
import re
import string
import sys
from css_properties import parse_css_model

# module_path = evita/visuals/css
module_path = os.path.dirname(os.path.realpath(__file__))

root_path = os.path.normpath(os.path.join(module_path, os.pardir, os.pardir,
                                          os.pardir))
third_party_dir = os.path.normpath(os.path.join(root_path, 'third_party'))

# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2

templates_dir = os.path.normpath(os.path.join(module_path, 'templates'))

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
# Generators
#
class Generator(object):

    def __init__(self, options):
        self._jinja_env = initialize_jinja_env(options.cache_directory)
        self._output_dir = options.output_directory

    def generate(self, model):
        template_names = [
            'properties.cc',
            'properties.h',
            'properties_forward.h',
            'values.cc',
            'values.h',
            'values_forward.h',
        ]
        context = self.make_context(model)
        for template_name in template_names:
            template = self._jinja_env.get_template(template_name)
            output_path = os.path.join(self._output_dir, template_name)
            with open(output_path, 'wt') as output:
                contents = template.render(context)
                output.write(contents)
            # Move to project to root to locate clang-format command
            os.chdir(root_path)
            os.system('clang-format -i %s' % output_path)

    def make_context(self, model):
        return {
            'keywords': model.keywords,
            'primitives': self.make_primitives(model),
            'properties': self.make_properties(model),
            'types': self.make_types(model),
        }

    def make_primitives(self, model):
        return model.primitive_types

    def make_properties(self, model):
        properties = []
        for css_property in model.properties:
            properties.append({
                'Name': css_property.name,
                'Parameter': 'const %s&' % css_property.name,
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
