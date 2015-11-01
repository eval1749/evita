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

from idl_definitions import Visitor
import idl_types
from idl_types import IdlType

IDL_TO_JS_TYPE_MAP = {
    'DOMString': 'string',
    'DOMTimeStamp': 'Date',
    'RangeCase': 'Range.Case',
    'RegExpInit': 'Editor.RegExpInit',
    'any': '*',
    'boolean': 'boolean',
    'byte': 'number',
    'double': 'number',
    'float': 'number',
    'long': 'number',
    'long long': 'number',
    'octet': 'number',
    'short': 'number',
    'unsigned long': 'number',
    'unsigned long long': 'number',
    'unsigned short': 'number',
}

NAMESPACE_MAP = {
    'AbstractFile': 'Os',
    'File': 'Os',
}


NON_NULL_JS_TYPE_SET = frozenset([
    'boolean',
    'number',
    'string',
    'void',
])


# TODO(eval1749) We should share |set_global_type_info()| with
# "code_generator_glue.py"
def set_global_type_info(info_provider):
    interfaces_info = info_provider.interfaces_info
    idl_types.set_ancestors(interfaces_info['ancestors'])
    IdlType.set_callback_interfaces(interfaces_info['callback_interfaces'])
    IdlType.set_dictionaries(interfaces_info['dictionaries'])
    IdlType.set_enums(info_provider.enumerations)
    IdlType.set_implemented_as_interfaces(
        interfaces_info['implemented_as_interfaces'])
    IdlType.set_garbage_collected_types(
        interfaces_info['garbage_collected_interfaces'])
    IdlType.set_will_be_garbage_collected_types(
        interfaces_info['will_be_garbage_collected_interfaces'])


# TODO(eval1749) We should share |depends_on_union_types()| with
# "code_generator_glue.py"
def depends_on_union_types(idl_type):
    """Returns true when a given idl_type depends on union containers
    directly.
    """
    if idl_type.is_union_type:
        return True
    if idl_type.is_array_or_sequence_type:
        return idl_type.element_type.is_union_type
    return False


# TODO(eval1749) We should share |TypedefResolver()| with
# "code_generator_glue.py"
class TypedefResolver(Visitor):

    def __init__(self, info_provider):
        self.info_provider = info_provider

    def resolve(self, definitions, definition_name):
        """Traverse definitions and resolves typedefs with the actual types."""
        self.typedefs = {}
        for name, typedef in self.info_provider.typedefs.iteritems():
            self.typedefs[name] = typedef.idl_type
        self.additional_includes = set()
        definitions.accept(self)
        self._update_dependencies_include_paths(definition_name)

    def _update_dependencies_include_paths(self, definition_name):
        interface_info = self.info_provider.interfaces_info[definition_name]
        dependencies_include_paths = interface_info[
            'dependencies_include_paths']
        for include_path in self.additional_includes:
            if include_path not in dependencies_include_paths:
                dependencies_include_paths.append(include_path)

    def _resolve_typedefs(self, typed_object):
        """Resolve typedefs to actual types in the object."""
        for attribute_name in typed_object.idl_type_attributes:
            try:
                idl_type = getattr(typed_object, attribute_name)
            except AttributeError:
                continue
            if not idl_type:
                continue
            resolved_idl_type = idl_type.resolve_typedefs(self.typedefs)
            if depends_on_union_types(resolved_idl_type):
                self.additional_includes.add(
                    self.info_provider.include_path_for_union_types)
            # Need to re-assign the attribute, not just mutate idl_type, since
            # type(idl_type) may change.
            setattr(typed_object, attribute_name, resolved_idl_type)

    def visit_typed_object(self, typed_object):
        self._resolve_typedefs(typed_object)


class CodeGeneratorJS(object):

    def __init__(self, info_provider, cache_dir, output_dir):
        self.info_provider = info_provider
        self.jinja_env = initialize_jinja_env(cache_dir)
        self.typedef_resolver = TypedefResolver(info_provider)
        self.output_dir = output_dir
        set_global_type_info(info_provider)

    def generate_code(self, module_definitions):
        definitions = module_definitions['dom']
        if len(definitions.interfaces) == 0:
            return [self.generate_code_for_dictionary(dictionary)
                    for dictionary in definitions.dictionaries.values()]
        else:
            return [self.generate_code_for_interface(interface, definitions)
                    for interface in definitions.interfaces.values()]

    def generate_code_for_dictionary(self, dictionary):
        template = self.jinja_env.get_template('dictionary.js')
        context = dictionary_context(dictionary)
        return {
            'contents': template.render(context),
            'file_name': '%s_externs.js' % dictionary.name,
        }

    def generate_code_for_interface(self, interface, definitions):
        template = self.jinja_env.get_template('interface.js')
        context = interface_context(interface, definitions)
        return {
            'contents': template.render(context),
            'file_name': '%s_externs.js' % interface.name,
        }


def attribute_context(attribute):
    return {
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
    if not interface.constructors and not interface.custom_constructors:
        return []
    contents = function_context(interface.constructors +
                                interface.custom_constructors)
    contents['parent_name'] = parent_name(interface.parent)
    contents['name'] = interface.name
    return [contents]


def dictionary_context(dictionary):
    if 'JsNamespace' in dictionary.extended_attributes:
        namespace = dictionary.extended_attributes['JsNamespace'] + '.'
    else:
        namespace = ''

    return {
        'dictionary_members': [dictionary_member_context(member)
                               for member in dictionary.members],
        'dictionary_name': dictionary.name,
        'namespace': namespace,
    }


def dictionary_member_context(member):
    if member.default_value:
        idl_type = member.idl_type
        # idl_type.is_nullable = True
    else:
        idl_type = member.idl_type
    return {
        'default_value': member.default_value,
        'name': member.name,
        'type': type_string(idl_type),
    }


def enumeration_context(enumeration):
    # FIXME: Handle empty string value. We don't have way to express empty
    # string as JS externs as of March 2014.
    return {
        'name': enumeration.name,
        'entries': [{'name': value.upper(), 'value': value}
                    for value in sorted(enumeration.values) if value],
    }


def function_context(functions):
    parameters_list = [function.arguments for function in functions]
    max_arity = max(map(len, parameters_list))
    normalized_parameters_list = [
        parameters + [None] * (max_arity - len(parameters))
        for parameters in parameters_list]
    parameters = [overloaded_parameter(overloaded_parameters)
                  for overloaded_parameters
                  in zip(*normalized_parameters_list)]
    return_type_strings = [type_string(function.idl_type)
                           for function in functions if function.idl_type]
    return {
        'is_static': functions[0].is_static,
        'name': functions[0].name,
        'parameters': parameters,
        'return_type': union_type_string(return_type_strings),
    }


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


def interface_context(interface, definitions):
    callback_context_list = [
        callback_context(callback_function)
        for callback_function in definitions.callback_functions.values()]

    dictionary_context_list = [
        dictionary_context(dictionary)
        for dictionary in definitions.dictionaries.values()]

    enumeration_context_list = [
        enumeration_context(enumeration)
        for enumeration in definitions.enumerations.values()]

    if 'JsNamespace' in interface.extended_attributes:
        namespace = interface.extended_attributes['JsNamespace'] + '.'
    else:
        namespace = ''

    attribute_context_list = [attribute_context(attribute)
                              for attribute in interface.attributes]
    constant_context_list = [constant_context(constant)
                             for constant in interface.constants]

    method_context_list = [
        function_context(list(functions))
        for name, functions in
        groupby(interface.operations, lambda operation: operation.name)
    ]

    # Context for tempaltes
    return {
        'attributes': sort_context_list(attribute_context_list),
        'callbacks': sort_context_list(callback_context_list),
        'constants': sort_context_list(constant_context_list),
        'constructors': constructor_context_list(interface),
        'dictionaries': sort_context_list(dictionary_context_list),
        'enumerations': sort_context_list(enumeration_context_list),
        'interfaces': interface_context_list(interface),
        'interface_name': namespace + interface.name,
        'methods': sort_context_list(method_context_list),
        'namespace': namespace,
    }


def interface_context_list(interface):
    if interface.constructors or interface.custom_constructors:
        return []
    return [{'name': interface.name,
             'parent_name': parent_name(interface.parent)}]


def overloaded_parameter(overloaded_parameters):
    parameters = [parameter for parameter in overloaded_parameters
                  if parameter]
    is_optional = len(parameters) != len(overloaded_parameters) or \
        any(parameter.is_optional for parameter in parameters)
    names = sorted(set([parameter.name for parameter in parameters]))
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


def parent_name(name):
    if name in NAMESPACE_MAP:
        return NAMESPACE_MAP[name] + '.' + name
    return name


def sort_context_list(context_list):
    return sorted(context_list, key=lambda context: context['name'])


def type_string(idl_type):
    base_type_string = type_string_without_nullable(idl_type)
    if idl_type.is_union_type:
        # Union type already handles nullable.
        return base_type_string
    assert base_type_string[0] != '!' and base_type_string[0] != '?', idl_type
    if base_type_string in NON_NULL_JS_TYPE_SET:
        if idl_type.is_nullable:
            return '?' + base_type_string
        return base_type_string
    if idl_type.is_nullable:
        return base_type_string
    return '!' + base_type_string


def type_string_without_nullable(idl_type):
    # TODO(eval1749): Once Blink IDL parser support Promise type, we should
    # encode return type.
    if idl_type.name == 'Promise':
        return 'Promise.<?>'
    if idl_type.is_nullable:
        return type_string_without_nullable(idl_type.inner_type)
    if idl_type.is_union_type:
        return '|'.join(sorted(type_string(member_type)
                               for member_type in idl_type.member_types))
    if idl_type.is_array or idl_type.is_sequence or \
       idl_type.is_array_or_sequence_type:
        return 'Array.<%s>' % type_string(idl_type.element_type)
    assert idl_type.base_type, idl_type
    if idl_type.base_type in IDL_TO_JS_TYPE_MAP:
        return IDL_TO_JS_TYPE_MAP[idl_type.base_type]
    return idl_type.name


def union_type_string(type_strings):
    type_string_list = sorted(set(type_strings))
    return '|'.join(type_string_list)
