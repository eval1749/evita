# Copyright (c) 2015 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from optparse import OptionParser
import os
import re
import string
import sys

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
# CSS Value Types
#
PRIMITIVE_TYPES = frozenset([
    'background',
    'border',
    'color',
    'length',
    'margin',
    'padding',
    'percentage',
])


class CssType(object):

    def __init__(self, text):
        self._name = capitalize(text)
        self._text = text
        self._underscore = text.replace('-', '_')

    @property
    def initial_value(self):
        return '/* no initial value for %s */' % self._name

    @property
    def is_compound(self):
        return False

    @property
    def is_enum(self):
        return False

    @property
    def is_keyword(self):
        return False

    @property
    def is_primitive(self):
        return False

    @property
    def members(self):
        return []

    @property
    def name(self):
        return self._name

    @property
    def text(self):
        return self._text

    @property
    def underscore(self):
        return self._underscore

    def to_parameter_type(self):
        return 'const %s&' % self.name

    def __cmp__(self, other):
        return cmp(self.name, other.name)

    def __str__(self):
        return '%s(%s)' % (self.__class__.__name__, self.name)


class CssCompoundType(CssType):

    def __init__(self, name, members):
        super(CssCompoundType, self).__init__(name)
        if not members[0].is_keyword:
            raise Exception('No initial value for %s' % name)
        self._initial_value = members[0].name
        self._members = sorted(members)

    @property
    def initial_value(self):
        return self._initial_value

    @property
    def is_compound(self):
        return True

    @property
    def members(self):
        return self._members


class CssEnumType(CssType):

    def __init__(self, name, members):
        super(CssEnumType, self).__init__(name)
        self._initial_value = members[0].name
        self._members = sorted(members)

    @property
    def initial_value(self):
        return self._initial_value

    @property
    def members(self):
        return self._members

    @property
    def is_enum(self):
        return True


class CssKeywordType(CssType):

    def __init__(self, text):
        super(CssKeywordType, self).__init__(text)
        self._text = text

    @property
    def is_keyword(self):
        return True

    @property
    def text(self):
        return self._text

    def to_parameter_type(self):
        return '/* KEYWORD %s */' % self.text


class CssPrimitiveType(CssType):

    def __init__(self, name):
        super(CssPrimitiveType, self).__init__(name)

    @property
    def is_primitive(self):
        return True


class CssPropty(object):

    def __init__(self, text, css_type):
        self._css_type = css_type
        self._name = capitalize(text)
        self._text = text
        self._underscore = text.replace('-', '_')

    @property
    def css_type(self):
        return self._css_type

    @property
    def name(self):
        return self._name

    @property
    def text(self):
        return self._text

    @property
    def underscore(self):
        return self._underscore


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
            'primitives': self.make_primitives(),
            'properties': self.make_properties(model),
            'types': self.make_types(model),
        }

    def make_primitives(self):
        return [
            {
                'Name': capitalize(name),
                'name': name,
            }
            for name in sorted([name for name in PRIMITIVE_TYPES])
        ]

    def make_properties(self, model):
        properties = []
        for css_property in model['properties']:
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
        return [self.make_type(css_type) for css_type in model['types']]


######################################################################
#
# Parser
#
def parse_model(lines):
    model = dict()
    model['properties'] = []
    model['types'] = []
    for raw_line in lines:
        line = raw_line.lstrip().rstrip()
        if len(line) == 0 or line[0] == '#':
            continue
        tokens = line.split(' ')
        property_name = tokens[0].replace(':', '')
        css_type = parse_type(property_name, tokens[1:])
        model['types'].append(css_type)
        css_property = CssPropty(property_name, css_type)
        model['properties'].append(css_property)
    return model


def parse_type(property_name, tokens):
    """Parse 'property-name: token+' to CssType"""
    css_type_name = property_name
    if len(tokens) == 1:
        return parse_type_name(tokens[0])
    keywords = [CssKeywordType(token) for token in tokens if token[0] != '<']
    members = [parse_type_name(token) for token in tokens if token[0] == '<']
    if len(members) == 0:
        return CssEnumType(css_type_name, keywords)
    return CssCompoundType(css_type_name, keywords + members)


def parse_type_name(token):
    assert token[0] == '<'
    assert token[-1] == '>'
    name = token[1:-1]
    if name in PRIMITIVE_TYPES:
        return CssPrimitiveType(name)
    return CssType(name)


######################################################################
#
# Utility Functions
#
def capitalize(text):
    """Convert foo-bar-baz to FooBarBaz."""
    return string.capwords(text, '-').replace('-', '')


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
    model = parse_model(open(input_file_name, 'rt').readlines())
    Generator(options).generate(model)

if __name__ == '__main__':
    sys.exit(main())
