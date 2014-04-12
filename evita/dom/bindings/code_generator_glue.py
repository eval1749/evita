# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import defaultdict
from itertools import groupby
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

referenced_interface_names = set()

IDL_TO_GLUE_TYPE_MAP = {
    'DOMString': 'base::string16',
    'bool': 'bool',
    'byte': 'uint8_t',
    'double': 'double',
    'float': 'float',
    'long': 'int',
    'long long': 'int64_t',
    'octet': 'int',
    'short': 'int',
    'unsigned long': 'int',
    'unsigned long long': 'uint64_t',
    'unsigned short': 'uint16_t',
}


class CodeGeneratorGlue(object):
    def __init__(self, interfaces_info, cache_dir):
        interfaces_info = interfaces_info or {}
        self.interfaces_info = interfaces_info
        self.jinja_env = initialize_jinja_env(cache_dir)
        for interface_info in interfaces_info.values():
            interface_info['include_path'] = fix_include_path(
                interface_info['include_path'])

    def generate_code(self, definitions, interface_name):
        """Returns glue as text."""

        template_context = generate_template_context(
            definitions, interface_name, self.interfaces_info)
        cc_template = self.jinja_env.get_template('interface.cc')
        cc_text = cc_template.render(template_context)
        h_template = self.jinja_env.get_template('interface.h')
        h_text = h_template.render(template_context)
        return (cc_text, h_text)


def attribute_context(attribute):
    if 'ImplementedAs' in attribute.extended_attributes:
        cpp_name = attribute.extended_attributes['ImplementedAs']
    else:
        cpp_name = attribute.name

    return {
        'cpp_name': cpp_name,
        'is_read_only': attribute.is_read_only,
        'is_static': attribute.is_static,
        'name': attribute.name,
        'type': type_string(attribute.idl_type),
    }


def callback_context(callback):
    return {
        'parameters': [parameter_context(parameter)
                       for parameter in callback.arguments],
        'name': callback.name,
        'type': type_string(callback.idl_type),
    }


def constant_context(constant):
    return {
        'name': constant.name,
        'type': type_string(constant.idl_type),
        'value': constant.value,
    }


# We consolidate overloaded signatures into one function signature.
# See URL.idl for example.
def constructor_context_list(interface):
    if not interface.constructors  and not interface.custom_constructors:
        return []
    contents = function_context(interface.constructors +
               interface.custom_constructors)
    contents['parent_name'] = interface.parent
    return [contents]


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
    return_type_strings = [type_string(function.idl_type)
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


def generate_template_context(definitions, interface_name, interfaces_info):
    callback_context_list = sorted(
        [callback_context(callback_function)
         for callback_function in definitions.callback_functions.values()],
        key=lambda context: context['name'])

    enumeration_context_list = sorted(
        [enumeration_context(enumeration)
         for enumeration in definitions.enumerations.values()],
        key=lambda context: context['name'])

    try:
        interface = definitions.interfaces[interface_name]
    except KeyError:
        raise Exception('%s not in IDL definitions' % interface_name)

    attribute_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [attribute_context(attribute) for attribute in interface.attributes])

    constant_context_list = [constant_context(constant)
                             for constant in interface.constants]

    method_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [function_context(list(functions))
         for name, functions in
         groupby(interface.operations, lambda operation: operation.name)])

    referenced_interface_names.add(interface_name)
    include_list = sorted([interfaces_info[name]['include_path']
                           for name in referenced_interface_names
                           if name in interfaces_info])

    if interface.parent:
        base_class_include = interfaces_info[interface.name]['include_path']
    else:
        base_class_include = None

    need_class_template = \
        any([attribute.is_static for attribute in interface.attributes]) or \
        any([operation.is_static for operation in interface.operations])

    need_instance_template = \
        any([not attribute.is_static for attribute in interface.attributes]) or \
        any([not operation.is_static for operation in interface.operations])

    return {
      'attributes': sort_context_list(attribute_context_list),
      'callbacks': sort_context_list(callback_context_list),
      'class_name': interface_name + 'Class',
      'base_class_include': base_class_include,
      'constants': sort_context_list(constant_context_list),
      'constructors': constructor_context_list(interface),
      'enumerations': enumeration_context_list,
      'need_class_template': need_class_template,
      'need_instance_template': need_instance_template,
      'includes': include_list,
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
    string = type_string(parameter.idl_type)
    return string + '=' if is_optional else string


def sort_context_list(context_list):
    return sorted(context_list, key=lambda context: context['name'])


def type_string(idl_type):
    if idl_type.is_union_type:
        return union_type_string([type_string(member_type)
                                  for member_type in idl_type.member_types])
    type_str = IDL_TO_GLUE_TYPE_MAP.get(idl_type.base_type, idl_type.base_type)
    if type_str == 'void':
        return ''
    referenced_interface_names.add(type_str)
    if idl_type.is_array or idl_type.is_sequence:
        return 'const std::vector<%s>&' % type_str
    if (idl_type.is_nullable):
        return 'v8_glue::Nullable<%s>' % type_str
    return type_str


def underscore(text):
    result =''
    start = True
    for ch in text:
        if ch >= 'A' and ch <= 'Z':
            if not start:
                result += '_'
            start = False
            result += chr(ord(ch) - ord('A') + ord('a'))
        else:
            result += ch
    return result


def union_type_string(type_strings):
    type_string_list = list(set(type_strings))
    type_string_list.sort()
    return '|'.join(type_string_list)


# Convert lower case cample, lowerCaseCamel, to upper case came, UpperCaseCamel
def upper_camel_case(lower_camel_case):
    return lower_camel_case[0].upper() + lower_camel_case[1:]
