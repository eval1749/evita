# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import string

######################################################################
#
# CSS Value Types
#
PRIMITIVE_TYPE_NAMES = frozenset([
    'background',
    'border',
    'color',
    'length',
    'padding',
    'percentage',
    'string',
])


PRIMITIVE_TYPES = dict()


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
        self._property_id = -1
        self._name = capitalize(text)
        self._text = text
        self._underscore = text.replace('-', '_')

    @property
    def css_type(self):
        return self._css_type

    @property
    def property_id(self):
        if self._property_id < 0:
            raise Exception('%s has no id' % self.name)
        return self._property_id

    def set_property_id(self, new_property_id):
        if self._property_id >= 0:
            raise Exception('%s already has id' % self.name)
        if new_property_id < 0:
            raise Exception('Invalid %d for %s' % (new_property_id, self.name))
        self._property_id = new_property_id

    @property
    def name(self):
        return self._name

    @property
    def text(self):
        return self._text

    @property
    def underscore(self):
        return self._underscore


class Model(object):

    def __init__(self, properties, types):
        self._primitive_types = [
            {
                'Name': capitalize(name),
                'name': name,
                'file_name': name.replace('-', '_'),
            }
            for name in sorted([name for name in PRIMITIVE_TYPE_NAMES])
        ]
        self._properties = properties
        self._types = types

    @property
    def primitive_types(self):
        return self._primitive_types

    @property
    def properties(self):
        return self._properties

    @property
    def types(self):
        return self._types


######################################################################
#
# Parser
#
def parse_css_model(lines):
    for name in PRIMITIVE_TYPE_NAMES:
        PRIMITIVE_TYPES[name] = CssPrimitiveType(name)
    properties = []
    types = []
    for raw_line in lines:
        line = raw_line.lstrip().rstrip()
        if len(line) == 0 or line[0] == '#':
            continue
        tokens = line.split(' ')
        property_name = tokens[0].replace(':', '')
        css_type = parse_type(property_name, tokens[1:])
        types.append(css_type)
        css_property = CssPropty(property_name, css_type)
        properties.append(css_property)
    properties.sort()
    property_id = 0
    for css_property in properties:
        css_property.set_property_id(property_id)
        property_id = property_id + 1
    return Model(properties, types)


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
        return PRIMITIVE_TYPES[name]
    return CssType(name)


######################################################################
#
# Utility Functions
#
def capitalize(text):
    """Convert foo-bar-baz to FooBarBaz."""
    if text[0].isdigit():
        return 'N' + text
    return string.capwords(text, '-').replace('-', '')
