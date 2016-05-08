# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import string

######################################################################
#
# CSS Value Types
# See "CSS Values and Units Module"
#
CSS_PRIMITIVE_TYPES = frozenset([
    # 'angle',
    # Note: We use |<color-value>| to avoid to conflict with property "color".
    'color-value',  # CSS Color Module
    # 'frequency',
    # 'image',
    # 'integer',
    'length',
    # 'number',
    'percentage',
    # 'position',  # CSS Backgrounds & Borders
    # 'resolution',
    'string',
    # 'time',
    # 'url',
])


CSS_WIDE_KEYWORDS = ['inherit', 'initial', 'unset']


CSS_ABSOLUTE_LENGTH_UNITS = frozenset([
    'cm', 'in', 'mm', 'pc', 'pt', 'px', 'q',
])
CSS_ABSOLUTE_ANGLE_UNITS = frozenset(['deg', 'grad', 'rad', 'turn'])
CSS_DURATION_UNITS = frozenset(['ms', 's'])
CSS_FREQUENCY_UNITS = frozenset(['Hz', 'kHz'])
CSS_RELATIVE_LENGTH_UNITS = frozenset([
    'ch', 'em', 'ex', 'rem', 'vh', 'xmax', 'vmin', 'vw',
])
CSS_RESOLUTION_UNITS = frozenset(['dpcm', 'dppx'])


class CssType(object):

    def __init__(self, text):
        self._name = capitalize(text)
        self._text = text
        self._underscore = text.replace('-', '_')

    def has(self, name):
        for member in self.members:
            if member.name == name:
                return True
        return False

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
        self._initial_value = members[0].name
        self._members = members

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
        self._members = members

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

    @property
    def is_keyword(self):
        return True

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

    def __cmp__(self, other):
        return cmp(self.name, other.name)

    def __str__(self):
        return '%s(%s)' % (self.__class__.__name__, self.name)


class Model(object):

    def __init__(self, keywords, properties, types):
        self._primitive_types = [
            {
                'Name': capitalize(name),
                'name': name,
                'file_name': name.replace('-', '_'),
            }
            for name in sorted([name for name in CSS_PRIMITIVE_TYPES])
        ]
        self._keywords = [
            {
                'Name': capitalize(keyword),
                'name': keyword.replace('-', '_'),
                'text': keyword,
            }
            for keyword in sorted(keyword for keyword in keywords)
        ]
        self._properties = properties
        self._types = types

    @property
    def keywords(self):
        return self._keywords

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
class Parser(object):

    def __init__(self):
        self._keywords = dict()
        self._properties = []
        self._types = dict()
        for keyword in CSS_WIDE_KEYWORDS:
            self.parse_type_keyword(keyword)
        for name in CSS_PRIMITIVE_TYPES:
            self._types['<%s>' % name] = CssPrimitiveType(name)

    def add_keyword(self, new_keyword):
        self._keywords[new_keyword.text] = new_keyword
        return new_keyword

    def add_type(self, new_type):
        if new_type.name[0] == '<':
            self._types[new_type.text] = new_type
        else:
            self._types[new_type.name] = new_type
        return new_type

    def make_model(self):
        self._properties.sort()
        # Note: properties_id=0 is "Invalid" property.
        next_property_id = 1
        for css_property in self._properties:
            css_property.set_property_id(next_property_id)
            next_property_id = next_property_id + 1
        keywords = sorted([keyword for keyword in self._keywords])
        types = sorted([value for value in self._types.values()
                        if value.name[0] != '<'])
        return Model(keywords, self._properties, types)

    def parse_line(self, line):
        tokens = line.split(' ')
        name = tokens[0].replace(':', '')
        css_type = self.parse_tokens(name, tokens[1:])
        if name[0] == '<':
            return
        self._properties.append(CssPropty(name, css_type))

    def parse_lines(self, lines):
        for raw_line in lines:
            line = raw_line.lstrip().rstrip()
            if len(line) == 0 or line[0] == '#':
                continue
            self.parse_line(line)
        return self.make_model()

    def parse_tokens(self, property_name, tokens):
        """Parse 'property-name: token+' to CssType"""
        css_type_name = property_name
        members = []
        for token in tokens:
            if token == 'currentColor':
                token = 'current-color'
            if token[0] != '<':
                if token in CSS_WIDE_KEYWORDS:
                    raise Exception('CSS wide keyword %s' % token)
                members.append(self.parse_type_keyword(token))
                continue
            name = token
            if not(name in self._types):
                raise Exception('Undefined type %s' % name)
            css_type = self._types[name]
            if css_type.is_primitive:
                members.append(css_type)
                continue
            for member in css_type.members:
                members.append(member)

        if css_type_name[0] != '<':
            for keyword in CSS_WIDE_KEYWORDS:
                members.append(self._keywords[keyword])
        for member in members:
            if not isinstance(member, CssEnumType):
                return self.add_type(CssCompoundType(css_type_name, members))
        return self.add_type(CssEnumType(css_type_name, members))

    def parse_type_keyword(self, keyword):
        if keyword in self._keywords:
            return self._keywords[keyword]
        return self.add_keyword(CssKeywordType(keyword))


def parse_css_model(lines):
    return Parser().parse_lines(lines)


######################################################################
#
# Utility Functions
#
def capitalize(text):
    """Convert foo-bar-baz to FooBarBaz."""
    if text[0].isdigit():
        return 'N' + text
    return string.capwords(text, '-').replace('-', '')
