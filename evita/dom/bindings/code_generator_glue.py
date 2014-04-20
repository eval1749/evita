# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import defaultdict
from itertools import chain, groupby
import os
import posixpath
import sys

module_path = os.path.dirname(os.path.realpath(__file__))
blink_path = os.path.normpath(os.path.join(module_path, os.pardir, os.pardir))
third_party_dir = os.path.normpath(os.path.join(blink_path, os.pardir))
templates_dir = module_path

# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2

import idl_types
from idl_types import IdlType

global_has_gc_member = False
global_has_nullable = False
global_has_optional = False
global_referenced_interface_names = set()
global_definitions = {}
global_interfaces_info = {}

# TODO(yosi) Once all interfaces have IDL file. We should get rid of
# |KNOWN_INTERFACE_NAMES|.
KNOWN_INTERFACE_NAMES = {
    'Form': 'evita/dom/forms/form.h',
}
global_known_interface_names = set()

class GlueType(object):
    def __init__(self, idl_type, cpp_name, is_pointer=False,
                 is_collectable=False, is_by_value=True):
        self.cpp_name = cpp_name
        self.idl_type = idl_type
        self.is_by_value = is_by_value
        self.is_collectable = is_collectable
        self.is_nullable = idl_type.is_nullable
        self.is_pointer = is_pointer

    def declare_str(self):
        if self.idl_type.is_union_type:
            raise "Union type isn't supported."
        if self.idl_type.is_array or self.idl_type.is_sequence:
            return 'std::vector<%s>' % self.cpp_name
        if self.is_collectable:
            global global_has_gc_member
            global_has_gc_member = True
            return 'gc::Member<%s>' % self.cpp_name
        if self.is_pointer:
            return self.cpp_name + '*'
        return self.cpp_name

    def getter(self, variable_name):
        if self.is_collectable:
            return variable_name + '.get()'
        return variable_name

    def parameter_str(self):
        if self.idl_type.is_union_type:
            raise "Union type isn't supported."
        if self.idl_type.is_array or self.idl_type.is_sequence:
            return 'const std::vector<%s>&' % self.cpp_name
        if self.is_collectable:
            if self.idl_type.is_nullable:
                global_has_nullable = True
                return 'v8_glue::Nullable<%s>' % self.cpp_name
            return self.cpp_name + '*'
        if self.is_pointer:
            return self.cpp_name + '*'
        return self.cpp_name

    def return_str(self):
        if self.idl_type.is_union_type:
            raise "Union type isn't supported."
        if self.idl_type.is_array or self.idl_type.is_sequence:
            return 'std::vector<%s>' % self.cpp_name
        if self.is_collectable:
            return self.cpp_name + '*'
        if self.is_pointer:
            return self.cpp_name + '*'
        return self.cpp_name

IDL_TO_GLUE_TYPE_MAP = {
    'DOMString': GlueType(IdlType('DOMString'), 'base::string16',
                          is_by_value=False),
    'bool': GlueType(IdlType('bool'), 'bool'),
    'byte': GlueType(IdlType('bool'), 'uint8_t'),
    'double': GlueType(IdlType('double'), 'double'),
    'float': GlueType(IdlType('float'), 'float'),
    'long': GlueType(IdlType('long'), 'int'),
    'long long': GlueType(IdlType('long long'), 'int64_t'),
    'octet': GlueType(IdlType('octet'), 'int8t'),
    'short': GlueType(IdlType('short'), 'int16_t'),
    'unsigned long': GlueType(IdlType('unsigned long'), 'int'),
    'unsigned long long': GlueType(IdlType('unsigned long long'), 'uint64_t'),
    'unsigned short': GlueType(IdlType('unsigned short'), 'uint16_t'),
}

def to_glue_type(idl_type):
    type_name = idl_type.base_type
    if type_name in IDL_TO_GLUE_TYPE_MAP:
        return IDL_TO_GLUE_TYPE_MAP[type_name]
    if type_name in global_interfaces_info:
        global_referenced_interface_names.add(type_name)
        return GlueType(idl_type, type_name, is_collectable=True)
    if type_name in KNOWN_INTERFACE_NAMES:
        global_known_interface_names.add(type_name)
        return GlueType(idl_type, type_name, is_collectable=True)
    return GlueType(idl_type, type_name)


class CodeGeneratorGlue(object):
    def __init__(self, interfaces_info, cache_dir):
        interfaces_info = interfaces_info or {}
        self.interfaces_info = interfaces_info
        global global_interfaces_info
        global_interfaces_info = interfaces_info
        self.jinja_env = initialize_jinja_env(cache_dir)
        for interface_info in interfaces_info.values():
            interface_info['include_path'] = fix_include_path(
                interface_info['include_path'])

    def dictionary_context(self, dictionary):
        context = generate_dictionary_context(dictionary, self.interfaces_info)
        return ('dictionary', context, dictionary.name, dictionary.name)

    def generate_contents(self, template_name, template_context):
        return self.jinja_env.get_template(template_name) \
            .render(template_context)

    def generate_cc_h(self, template_name, template_context, name, file_name):
        return [
            {
                'contents': self.generate_contents(
                    template_name + extension, template_context),
                'file_name': file_name + extension,
            }
            for extension in ['.cc', '.h']
        ]

    def generate_dummy_files_if_needed(self, definitions, interface_name):
        if definitions.dictionaries:
            return []
        return [
            {
                'contents': '#error "should not use"\n',
                'file_name': interface_name + 'Init' + extension,
            }
            for extension in ['.cc', '.h']
        ]

    def generate_code(self, definitions, interface_name):
        global global_definitions
        global_definitions = definitions
        return list(chain(*[
            self.generate_cc_h(template_name, template_context, name, type)
            for (template_name, template_context, name, type) in
            [self.dictionary_context(dictionary)
             for dictionary in definitions.dictionaries.values()] +
            # interface context must be generated after dictionary context
            # for include files.
            [self.interface_context(definitions, interface_name)]
        ])) + \
        list(chain(self.generate_dummy_files_if_needed(definitions,
                                                       interface_name)))

    def interface_context(self, definitions, interface_name):
        context = generate_interface_context(
            definitions, interface_name, self.interfaces_info)
        return ('interface', context, interface_name, interface_name + 'Class')

######################################################################
#
# Dictionary
#
def dictionary_member_context(member):
    if 'ImplementedAs' in member.extended_attributes:
        cpp_name = member.extended_attributes['ImplementedAs']
    else:
        cpp_name = underscore(member.name)

    if member.idl_type.is_nullable:
        global global_has_nullable
        global_has_nullable = True

    glue_type = to_glue_type(member.idl_type)

    return {
        'cpp_name': cpp_name,
        'declare_type': glue_type.declare_str(),
        'default_value': member.default_value,
        'getter': glue_type.getter(cpp_name + '_'),
        'has_default_value': member.default_value != None,
        'is_nullable': member.idl_type.is_nullable,
        'name': member.name,
        'parameter_type': glue_type.return_str(),
        'return_type': glue_type.return_str(),
        'type': member.idl_type.base_type,
    }

    type_str = IDL_TO_GLUE_TYPE_MAP.get(idl_type.base_type, idl_type.base_type)


def generate_dictionary_context(dictionary, interfaces_info):
    global global_has_nullable
    global_has_nullable = False

    global global_referenced_interface_names
    global_referenced_interface_names = set()

    member_context_list = map(dictionary_member_context, dictionary.members)
    if dictionary.parent:
        base_class_include = '...TODO...'
    else:
        base_class_include = 'evita/dom/dictionary.h'

    class_references = list(global_referenced_interface_names)
    cc_include_paths = map(interface_name_to_include_path,
                           global_referenced_interface_names)
    cc_include_paths.append('evita/dom/converter.h')
    if global_has_nullable:
        cc_include_paths.append('evita/v8_glue/nullable.h')
    cc_include_paths.append('../../v8_strings.h')

    for name in global_known_interface_names:
        class_references.append(name)
        cc_include_paths.append(KNOWN_INTERFACE_NAMES[name])

    h_include_paths = []
    if global_has_gc_member:
        h_include_paths.append('evita/gc/member.h')

    return {
        'base_class_include': base_class_include,
        'cc_include_paths': sorted(cc_include_paths),
        'class_references': sorted(class_references),
        'has_gc_member': global_has_gc_member,
        'h_include_paths': sorted(h_include_paths),
        'members': sort_context_list(member_context_list),
        'name': dictionary.name,
        'parent_name': dictionary.parent,
    }


######################################################################
#
# Interface
#
def attribute_context(attribute):
    if 'ImplementedAs' in attribute.extended_attributes:
        cpp_name = attribute.extended_attributes['ImplementedAs']
    else:
        cpp_name = attribute.name
    glue_type = to_glue_type(attribute.idl_type)
    return {
        'cpp_name': cpp_name,
        'is_read_only': attribute.is_read_only,
        'is_static': attribute.is_static,
        'name': attribute.name,
        'type': glue_type.parameter_str()
    }


def callback_context(callback):
    return {
        'parameters': [parameter_context(parameter)
                       for parameter in callback.arguments],
        'name': callback.name,
        'type': parameter_type_string(callback.idl_type),
    }


def constant_context(constant):
    return {
        'name': constant.name,
        'type': parameter_type_string(constant.idl_type),
        'value': constant.value,
    }


# We consolidate overloaded signatures into one function signature.
# See URL.idl for example.
def constructor_context_list(interface):
    if not interface.constructors  and not interface.custom_constructors:
        return []
    return map(constructor_context, interface.constructors)

def constructor_context(constructor):
    parameters = map(constructor_parameter, constructor.arguments)
    return { 'parameters': parameters }

def constructor_parameter(parameter):
    glue_type = to_glue_type(parameter.idl_type)
    return {
        'cpp_name': underscore(parameter.name),
        'declare_type': glue_type.declare_str(),
    }


def enumeration_context(enumeration):
    # FIXME: Handle empty string value. We don't have way to express empty
    # string as Glue externs as of March 2014.
    return {
        'name': enumeration.name,
        'entries': [{'name': value.upper(), 'value': value}
                    for value in sorted(enumeration.values) if value],
    }


def fix_include_path(path):
    return os.path.join(os.path.dirname(path),
        underscore(os.path.basename(path))).replace('\\', '/')


def function_context(functions):
    parameters_list = [function.arguments for function in functions]
    max_arity = max(map(len, parameters_list))
    normalized_parameters_list = [
        parameters + [None] * (max_arity - len(parameters))
        for parameters in parameters_list
    ]
    parameters = [overloaded_parameter(overloaded_parameters)
                  for overloaded_parameters
                  in zip(*normalized_parameters_list)]
    return_type_strings = [to_glue_type(function.idl_type).return_str()
                           for function in functions if function.idl_type]

    if 'ImplementedAs' in functions[0].extended_attributes:
        cpp_name = functions[0].extended_attributes['ImplementedAs']
    else:
        cpp_name = upper_camel_case(functions[0].name)

    return {
        'is_static': functions[0].is_static,
        'name': functions[0].name,
        'cpp_name': cpp_name,
        'parameters': parameters,
        'return_type': union_type_string(return_type_strings),
    }


def generate_interface_context(definitions, interface_name, interfaces_info):
    callback_context_list = [
        callback_context(callback_function)
        for callback_function in definitions.callback_functions.values()
    ]

    enumeration_context_list =[
        enumeration_context(enumeration)
        for enumeration in definitions.enumerations.values()
    ]

    try:
        interface = definitions.interfaces[interface_name]
    except KeyError:
        raise Exception('%s not in IDL definitions' % interface_name)

    attribute_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [attribute_context(attribute) for attribute in interface.attributes])

    constant_context_list = [constant_context(constant)
                             for constant in interface.constants]

    constructors = constructor_context_list(interface)

    method_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [function_context(list(functions))
         for name, functions in
         groupby(interface.operations, lambda operation: operation.name)])

    global_referenced_interface_names.add(interface_name)

    include_paths = map(interface_name_to_include_path,
                        global_referenced_interface_names)
    include_paths += [
        dictionary.name + '.h'
        for dictionary in definitions.dictionaries.values()
    ]
    include_paths.append('evita/dom/converter.h')
    include_paths.append('evita/dom/script_host.h')
    include_paths.append('evita/v8_glue/function_template_builder.h')
    if global_has_gc_member:
        include_paths.append('evita/gc/member.h')
    # TODO(yosi) We should include "array_buffer.h" if needed
    include_paths.append('gin/array_buffer.h')
    if global_has_nullable:
        include_paths.append('evita/v8_glue/nullable.h')
    if global_has_optional:
        include_paths.append('evita/v8_glue/optional.h')

    for name in global_known_interface_names:
        include_paths.append(KNOWN_INTERFACE_NAMES[name])

    if interface.parent:
        base_class_include = interfaces_info[interface.name]['include_path']
    else:
        base_class_include = None

    need_class_template = \
        any([attribute.is_static for attribute in interface.attributes]) or \
        any([operation.is_static for operation in interface.operations]) or \
        len(constructors) > 0

    need_instance_template = \
        any([not attribute.is_static for attribute in interface.attributes]) or \
        any([not operation.is_static for operation in interface.operations])

    return {
      'attributes': sort_context_list(attribute_context_list),
      'callbacks': sort_context_list(callback_context_list),
      'class_name': interface_name + 'Class',
      'base_class_include': base_class_include,
      'constants': sort_context_list(constant_context_list),
      'constructors': constructors,
      'enumerations': enumeration_context_list,
      'need_class_template': need_class_template,
      'need_instance_template': need_instance_template,
      'include_paths': sorted(include_paths),
      'interfaces': interface_context_list(interface),
      'interface_name': interface_name,
      'interface_parent': interface.parent,
      'methods': sort_context_list(method_context_list),
    }


def initialize_jinja_env(cache_dir):
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        # Bytecode cache is not concurrency-safe unless pre-cached:
        # if pre-cached this is read-only, but writing creates a race condition.
        bytecode_cache=jinja2.FileSystemBytecodeCache(cache_dir),
        keep_trailing_newline=True,  # newline-terminate generated files
        lstrip_blocks=True,  # so can indent control flow tags
        trim_blocks=True)
    return jinja_env


def interface_context_list(interface):
    if interface.constructors or interface.custom_constructors:
        return []
    return [{'name': interface.name, 'parent_name': interface.parent}]


def overloaded_parameter(overloaded_parameters):
    parameters = [parameter for parameter in overloaded_parameters
                  if parameter]
    is_optional = len(parameters) != len(overloaded_parameters) or \
                  any(parameter.is_optional for parameter in parameters)
    names = list(set([parameter.name for parameter in parameters]))
    names.sort()
    name = '_or_'.join(names)
    parameter_type_strings = [parameter_type_string(parameter, is_optional)
                              for parameter in parameters]
    return {
        'name': parameter_name(name, is_optional),
        'type': union_type_string(parameter_type_strings),
    }


def parameter_context(parameter):
    return {
        'name': parameter_name(parameter.name, parameter.is_optional),
        'type': parameter_type_string(parameter, parameter.is_optional),
    }


def parameter_name(name, is_optional):
    return 'opt_' + name if is_optional else name


def parameter_type_string(parameter, is_optional):
    glue_type = to_glue_type(parameter.idl_type)
    if is_optional:
        global global_has_optional
        global_has_optional = True
        return 'v8_glue::Optional<%s>' % glue_type.parameter_str()
    return glue_type.parameter_str()

######################################################################
#
# Common
#
def interface_name_to_include_path(interface_name):
    return global_interfaces_info[interface_name]['include_path']
                           
def sort_context_list(context_list):
    return sorted(context_list, key=lambda context: context['name'])


def underscore(text):
    result =''
    start = True
    for ch in text:
        if ch >= 'A' and ch <= 'Z':
            if not start:
                result += '_'
            result += chr(ord(ch) - ord('A') + ord('a'))
        else:
            result += ch
        start = False
    return result


# TODO(yosi) We should not use union_type_string
def union_type_string(type_strings):
    return '|'.join(type_strings)

# Convert lower case cample, lowerCaseCamel, to upper case came, UpperCaseCamel
def upper_camel_case(lower_camel_case):
    return lower_camel_case[0].upper() + lower_camel_case[1:]
