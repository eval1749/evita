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
templates_dir = os.path.normpath(os.path.join(module_path, 'templates'))

# jinja2 is in chromium's third_party directory.
# Insert at 1 so at front to override system libraries, and
# after path[0] == invoking script dir
sys.path.insert(1, third_party_dir)
import jinja2

from idl_definitions import Visitor
import idl_types
from idl_types import IdlType

global_has_gc_member = False
global_has_nullable = False
global_has_optional = False
global_referenced_dictionary_names = set()
global_referenced_interface_names = set()
global_definitions = {}
global_interfaces_info = {}

FILE_NAME_PREFIX = 'v8_glue_'

# TODO(eval1749) Once all interfaces have IDL file. We should get rid of
# |KNOWN_INTERFACE_NAMES|.
KNOWN_INTERFACE_NAMES = {
    'Document': 'evita/dom/text/document.h',
    # 'Range': 'evita/dom/text/range.h',
}
global_known_interface_names = set()

JS_INTERFACE_NAMES = {
    'Point': 'evita/dom/windows/point.h',
    'Rect': 'evita/dom/windows/rect.h',
}
global_js_interface_names = set()

def should_be_callback(idl_type):
    if idl_type.is_callback_function or idl_type.is_callback_interface:
        return True
    assert not idl_type.name.endswith('Callback')
    return False

# GlueType
class GlueType(object):
    def __init__(self, idl_type, cpp_name, is_by_value=True,
                 is_collectable=False, is_pointer=False, is_struct=False):
        self.cpp_name = cpp_name
        self.idl_type = idl_type
        self.is_by_value = is_by_value
        self.is_collectable = is_collectable
        self.is_nullable = idl_type.is_nullable
        self.is_pointer = is_pointer
        self.is_struct = is_struct
        if self.idl_type.is_array or self.idl_type.is_sequence or idl_type.is_array_or_sequence_type:
            element_type = to_glue_type(idl_type.element_type)
            self.element_typestr = element_type.return_str()
        else:
            self.element_typestr = None

    def declare_str(self):
        if self.idl_type.is_union_type:
            raise Exception("Can't use as member: " + str(self.idl_type))
        if self.element_typestr:
            return 'std::vector<%s>' % self.element_typestr
        if self.is_collectable:
            global global_has_gc_member
            global_has_gc_member = True
            return 'gc::Member<%s>' % self.cpp_name
        if self.is_pointer:
            return self.cpp_name + '*'
        return self.cpp_name

    def display_str(self):
        return str(self.idl_type)

    # Used for variable declaration of output parameter of |gin::ConvertFromV8|.
    def from_v8_str(self):
        if self.idl_type.is_union_type:
            members = [member for member in self.idl_type.idl_types()]
            return 'v8_glue::Either<' + ', '.join(to_glue_type(member_type).from_v8_str() for member_type in members[1:]) + '>'
        if self.element_typestr:
            return 'std::vector<%s>' % self.element_typestr
        if self.is_collectable:
            if self.idl_type.is_nullable:
                global global_has_nullable
                global_has_nullable = True
                return 'v8_glue::Nullable<%s>' % self.cpp_name
            return self.cpp_name + '* /* from_v8 is_collectable */'
        if self.is_pointer:
            return self.cpp_name + '* /* from_v8 is_pointer */'
        return self.cpp_name

    def parameter_str(self):
        if self.element_typestr:
            return 'const std::vector<%s>&' % self.element_typestr
        return self.return_str()

    def return_str(self):
        if self.idl_type.is_union_type:
            return self.declare_str()
        if self.element_typestr:
            return 'std::vector<%s>' % self.element_typestr
        if self.is_collectable:
            return self.cpp_name + '* /* return_str is_collectable */'
        if self.is_pointer:
            return self.cpp_name + '* /* return_str is_ponter */'
        return self.cpp_name

    def to_v8_str(self):
        if self.cpp_name == 'void':
            return self.cpp_name
        if self.is_collectable and self.idl_type.is_nullable:
            global global_has_nullable
            global_has_nullable = True
            return 'v8_glue::Nullable<%s>' % self.cpp_name
        return 'auto'


class CppType(object):
    def __init__(self, cpp_name, is_by_value=True, is_collectable=False,
                 is_pointer=False, is_struct=False):
        self.cpp_name = cpp_name
        self.is_by_value = is_by_value
        self.is_collectable = is_collectable
        self.is_pointer = is_pointer
        self.is_struct = is_struct

    def __str__(self):
        return 'CppType(' + self.cpp_name + \
            (' by_value' if self.is_by_value else '') + \
            (' collectable' if self.is_collectable else '') + \
            (' pointer' if self.is_pointer else '') + \
            (' struct' if self.is_pointer else '') + ')'


IDL_TO_CPP_TYPE_MAP = {
    # TODO(eval1749) We should have "SwitchValue.idl".
    'SwitchValue': CppType('domapi::SwitchValue', is_by_value=False),
    # TODO(eval1749) We should have "TabData.idl".
    'TabData': CppType('domapi::TabData', is_by_value=False),

    # Glue specific types
    'Alter': CppType('Alter'),
    'ArrayBufferView': CppType('gin::ArrayBufferView', is_by_value=False),
    'DataTransferData': CppType('DataTransferData', is_pointer=True),
    'EventListener': CppType('v8::Handle<v8::Object>'),
    # For Window.prototype.compute_
    'Point': CppType('domapi::FloatPoint'),
    # For Window.prototype.compute_
    'Rect': CppType('domapi::FloatRect'),
    # For Editor.runScript
    'RunScriptResult': CppType('v8::Handle<v8::Object>'),
    # For Editor.localizeText
    'StringDict': CppType('v8::Handle<v8::Object>'),
    'Style': CppType('v8::Handle<v8::Object>'),
    'Unit': CppType('Unit'),

    # V8 types
    'Function': CppType('v8::Handle<v8::Function>'),
    'Object': CppType('v8::Handle<v8::Object>'),
    'Promise': CppType('v8::Handle<v8::Promise>'),

    # IDL types
    'DOMString': CppType('base::string16', is_by_value=False),
    # DOMTimeStamp is defined in "core/dom/CommonDefinitions.idl".
    'DOMTimeStamp': CppType('TimeStamp'),
    'any': CppType('v8::Handle<v8::Value>'),
    'boolean': CppType('bool'),
    'byte': CppType('uint8_t'),
    'double': CppType('double'),
    'float': CppType('float'),
    'long': CppType('int'),
    'long long': CppType('int64_t'),
    'octet': CppType('int8t'),
    'short': CppType('int16_t'),
    'unsigned long': CppType('int'),
    'unsigned long long': CppType('uint64_t'),
    'unsigned short': CppType('uint16_t'),
    'void': CppType('void'),
}

# Map IDL type to Glue Type
def to_glue_type(idl_type):
    if idl_type.is_array or idl_type.is_sequence_type or idl_type.is_array_or_sequence_type:
        return GlueType(idl_type, idl_type.element_type.base_type)

    if idl_type.is_union_type:
        return GlueType(idl_type, idl_type.name)

    type_name = idl_type.base_type

    if type_name == None:
        raise 'to_glue_type(' + str(idl_type) + ')'

    if type_name in IDL_TO_CPP_TYPE_MAP:
        if type_name in JS_INTERFACE_NAMES:
            global_js_interface_names.add(type_name)
        cpp_type = IDL_TO_CPP_TYPE_MAP[type_name]
        return GlueType(idl_type, cpp_type.cpp_name,
                        is_by_value=cpp_type.is_by_value,
                        is_pointer=cpp_type.is_pointer)

    if type_name in KNOWN_INTERFACE_NAMES:
        global_known_interface_names.add(type_name)
        return GlueType(idl_type, type_name, is_collectable=True)

    if idl_type.is_dictionary:
        global_referenced_dictionary_names.add(type_name)
        return GlueType(idl_type, type_name, is_struct=True)

    if should_be_callback(idl_type):
        return GlueType(idl_type, 'v8::Handle<v8::Function>', is_struct=True)

    if idl_type.is_interface_type:
        assert idl_type.is_interface_type, idl_type
        assert not idl_type.name.endswith('Callback'), 'Should be callback ' + idl_type.name
        assert not idl_type.name.endswith('Init'), 'Should be dictionary ' + idl_type.name
        if type_name in global_interfaces_info:
            global_referenced_interface_names.add(type_name)
        return GlueType(idl_type, type_name, is_collectable=True)

    raise Exception('Unknown type: ' + str(idl_type))


# interface_info
#   component_dir   {'KeyboardEvent': 'dom', 'TextEncoder': 'dom', ...}

def set_global_type_info(info_provider):
    interfaces_info = info_provider.interfaces_info
    idl_types.set_ancestors(interfaces_info['ancestors'])
    IdlType.set_callback_interfaces(interfaces_info['callback_interfaces'])
    IdlType.set_dictionaries(interfaces_info['dictionaries'])
    IdlType.set_enums(info_provider.enumerations)
    IdlType.set_implemented_as_interfaces(interfaces_info['implemented_as_interfaces'])
    IdlType.set_garbage_collected_types(interfaces_info['garbage_collected_interfaces'])
    IdlType.set_will_be_garbage_collected_types(interfaces_info['will_be_garbage_collected_interfaces'])


def depends_on_union_types(idl_type):
    """Returns true when a given idl_type depends on union containers
    directly.
    """
    if idl_type.is_union_type:
        return True
    if idl_type.is_array_or_sequence_type:
        return idl_type.element_type.is_union_type
    return False


class TypedefResolver(Visitor):
    def __init__(self, info_provider):
        self.additional_includes = set()
        self.info_provider = info_provider
        self.typedefs = {}

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
        dependencies_include_paths = interface_info['dependencies_include_paths']
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


# interfaces_info contains:
#   ancestors
#   callback_interfaces
#   component_dirs
#   dictionaries
#   garbage_collected_interfaces
#   implemented_as_interfaces
#   (Interface name, starts with capitale letter)
#
#   Example:
# CompositionEvent {
#   'relative_dir': '../../../evita/dom/events',
#   'include_path': '../../../evita/dom/events/CompositionEvent.h',
#   'ancestors': ['UiEvent', 'Event'],
#   'dependencies_include_paths': [],
#   'parent': 'UiEvent',
#   'inherited_extended_attributes': {},
#   'dependencies_other_component_include_paths': [],
#   'dependencies_full_paths': [],
#   'implements_interfaces': [],
#   'referenced_interfaces': [],
#   'implemented_as': None,
#   'is_dictionary': False,
#   'is_callback_interface': False,
#   'dependencies_other_component_full_paths': [],
#   'full_path': 'D:\\w\\evita\\src\\evita\\dom\\events\\CompositionEvent.idl'
#  }
class CodeGeneratorGlue(object):
    def __init__(self, info_provider, cache_dir, output_dir):
        self.info_provider = info_provider
        self.jinja_env = initialize_jinja_env(cache_dir)
        self.output_dir = output_dir
        self.typedef_resolver = TypedefResolver(info_provider)
        set_global_type_info(info_provider)

        interfaces_info = info_provider.interfaces_info
        global global_interfaces_info
        global_interfaces_info = interfaces_info

        #for interface_info in interfaces_info.values():
        for key in interfaces_info.keys():
            if key[0] >= 'a':
                continue
            interface_info = interfaces_info[key]
            interface_info['include_path'] = fix_include_path(
                interface_info['include_path'])

    def generate_contents(self, context, extension):
        template_file_name = context['template_name'] + extension
        return self.jinja_env.get_template(template_file_name).render(context)

    def generate_cc_h(self, context):
        return [
            {
                'contents':  self.generate_contents(context,  extension),
                'file_name': context['output_name'] + extension,
            }
            for extension in ['.cc', '.h']
        ]

    # module_definitions = {'dom': idl_definitions.IdlDefinitions}
    def generate_code(self, module_definitions):
        definitions = module_definitions['dom']

        # Note: callback functions aren't visible out side defined IDL.
        IdlType.set_callback_interfaces(definitions.callback_functions)

        global global_definitions
        global_definitions = definitions
        return list(chain(*[
            self.generate_cc_h(context)
            for context in
            [dictionary_context(dictionary)
             for dictionary in definitions.dictionaries.values()] +
            # interface context must be generated after dictionary context
            # for include files.
            [interface_context(interface)
             for interface in definitions.interfaces.values()]
        ]))


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
        'default_value': cpp_value(member.default_value),
        'display_type': glue_type.display_str(),
        'from_v8_type': glue_type.from_v8_str(),
        'has_default_value': member.default_value != None,
        'is_nullable': member.idl_type.is_nullable,
        'name': member.name,
        'parameter_type': glue_type.parameter_str(),
        'return_type': glue_type.return_str(),
    }


def dictionary_context(dictionary):
    global global_has_nullable
    global_has_nullable = False

    global global_referenced_dictionary_names
    global_referenced_dictionary_names = set()

    global global_referenced_interface_names
    global_referenced_interface_names = set()

    member_context_list = map(dictionary_member_context, dictionary.members)
    if dictionary.parent:
        base_class_include = dictionary_name_to_include_path(dictionary.parent)
    else:
        base_class_include = 'evita/dom/dictionary.h'

    class_references = list(global_referenced_interface_names)

    cc_include_paths = map(interface_name_to_include_path,
                        filter(lambda name: 'include_path' in global_interfaces_info[name],
                               global_referenced_interface_names)) + \
                    map(dictionary_name_to_include_path,
                        global_referenced_dictionary_names)
    cc_include_paths.append('evita/dom/converter.h')
    if global_has_nullable:
        cc_include_paths.append('evita/v8_glue/nullable.h')
    cc_include_paths.append('../../v8_strings.h')

    for name in global_js_interface_names:
        class_references.append(name)
        cc_include_paths.append(JS_INTERFACE_NAMES[name])

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
        'h_include_paths': sorted(h_include_paths),
        'members': sort_context_list(member_context_list),
        'name': dictionary.name,
        'output_name': dictionary.name,
        'parent_name': dictionary.parent,
        'template_name': 'dictionary',
    }


######################################################################
#
# Interface
#
def attribute_context(attribute):
    if 'ImplementedAs' in attribute.extended_attributes:
        cpp_name = attribute.extended_attributes['ImplementedAs']
    else:
        cpp_name = underscore(attribute.name)

    if cpp_name == 'JavaScript':
        return {'cpp_name': cpp_name}

    glue_type = to_glue_type(attribute.idl_type)
    return {
        'cpp_name': cpp_name,
        'from_v8_type': glue_type.from_v8_str(),
        'is_read_only': attribute.is_read_only,
        'is_static': attribute.is_static,
        'name': attribute.name,
        'to_v8_type': glue_type.to_v8_str(),
    }


def callback_context(callback):
    glue_type = to_glue_type(callback.idl_type)
    return {
        'parameters': [parameter.name
                       for parameter in callback.arguments],
        'name': callback.name,
        'type': glue_type.to_v8_str(),
    }


def constant_context(constant):
    glue_type = to_glue_type(constant.idl_type)
    return {
        'name': constant.name,
        'type': glue_type.from_v8_str(),
        'value': constant.value,
    }


# IdlConstructor is derived class of IdlOperation.
# IdlConstructor.name = 'Constructor' | 'NamedConstructor'
def constructor_context_list(interface):
    name = interface.name
    return [
        # Using |new| operator
        {
            'cpp_name': 'new ' + name,
            'parameters': parameters
        }
        for parameters in expand_parameters(interface.constructors)
    ] + [
        # Custom constructor. It is used for validating parameters and
        # throw JavaScript exception for an invalid parameter.
        {
            'cpp_name': name + '::New' + name,
            'parameters': parameters
        }
        for parameters in expand_parameters(interface.custom_constructors)
    ]

def enumeration_context(enumeration):
    # FIXME: Handle empty string value. We don't have way to express empty
    # string as Glue externs as of March 2014.
    return {
        'name': enumeration.name,
        'entries': [{'name': value.upper(), 'value': value}
                    for value in sorted(enumeration.values) if value],
    }


def fix_include_path(path):
    return os.path.join(os.path.dirname(path).replace('../', ''),
        underscore(os.path.basename(path))).replace('\\', '/')


def function_context(functions):
    if 'ImplementedAs' in functions[0].extended_attributes:
        cpp_name = functions[0].extended_attributes['ImplementedAs']
    else:
        cpp_name = upper_camel_case(functions[0].name)

    if cpp_name == 'JavaScript':
        return {'cpp_name': cpp_name}

    parameters_list = expand_parameters(functions)
    assert parameters_list

    is_static = functions[0].is_static
    return_glue_type = to_glue_type(functions[0].idl_type)
    to_v8_type = return_glue_type.to_v8_str()

    signatures = [
        {
          'cpp_name': cpp_name,
          'is_static': is_static,
          'parameters': parameters,
          'to_v8_type': to_v8_type,
        }
        for parameters in parameters_list
    ]
    context = function_dispatcher(signatures)
    context['cpp_name'] = cpp_name
    context['is_static'] = is_static
    context['name'] = functions[0].name
    return context


def function_dispatcher(signatures):
    if not signatures:
        return {'dispatch': 'none'}
    max_arity = max([len(signature['parameters']) for signature in signatures])
    min_arity = min([len(signature['parameters']) for signature in signatures])
    if len(signatures) == 1:
        return {
            'dispatch': 'single',
            'max_arity': max_arity,
            'min_arity': min_arity,
            'cpp_name': signatures[0]['cpp_name'],
            'signature': signatures[0]
        }
    if len(set([len(signature['parameters']) for signature in signatures])) == \
       len(signatures):
        return {
            'dispatch': 'arity',
            'max_arity': max_arity,
            'min_arity': min_arity,
            'cpp_name': signatures[0]['cpp_name'],
            'signatures': signatures
        }
    raise Exception('NYI: type based dispatch')


def function_parameter(parameter):
    glue_type = to_glue_type(parameter.idl_type)
    return {
        'cpp_name': underscore(parameter.name),
        'display_type': glue_type.display_str(),
        'from_v8_type': glue_type.from_v8_str(),
    }


def interface_context(interface):
    if 'ImplementedAs' in interface.extended_attributes:
        if interface.extended_attributes['ImplementedAs'] == 'JavaScript':
            return {
                'INTERFACE_NAME': underscore(interface.name).upper(),
                'interface_name': interface.name,
                'output_name': interface.name,
                'template_name': 'js_interface',
            }

    callback_context_list = [
        callback_context(callback_function)
        for callback_function in global_definitions.callback_functions.values()
    ]

    enumeration_context_list =[
        enumeration_context(enumeration)
        for enumeration in global_definitions.enumerations.values()
    ]

    attribute_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [attribute_context(attribute) for attribute in interface.attributes])

    constant_context_list = [
        constant_context(constant)
        for constant in interface.constants
    ]

    constructor = function_dispatcher(constructor_context_list(interface))

    dictionaries = [
        {'name': dictionary.name}
        for dictionary in global_definitions.dictionaries.values()
    ]

    method_context_list = filter(
        lambda context: context['cpp_name'] != 'JavaScript',
        [function_context(list(functions))
         for name, functions in
         groupby(interface.operations, lambda operation: operation.name)])

    global_referenced_interface_names.add(interface.name)

    include_paths = map(interface_name_to_include_path,
                        filter(lambda name: 'include_path' in global_interfaces_info[name],
                               global_referenced_interface_names)) + \
                    map(dictionary_name_to_include_path,
                        global_referenced_dictionary_names)
    include_paths.append('base/logging.h')
    include_paths.append('evita/dom/converter.h')
    include_paths.append('evita/dom/script_host.h')
    include_paths.append('evita/v8_glue/function_template_builder.h')
    if global_has_gc_member:
        include_paths.append('evita/gc/member.h')
    # TODO(eval1749) We should include "array_buffer.h" if needed
    include_paths.append('gin/array_buffer.h')
    if global_has_nullable:
        include_paths.append('evita/v8_glue/nullable.h')
    if global_has_optional:
        include_paths.append('evita/v8_glue/optional.h')

    if constructor['dispatch'] != 'none':
        include_paths.append('evita/v8_glue/constructor_template.h')

    for name in global_js_interface_names:
        include_paths.append(JS_INTERFACE_NAMES[name])

    for name in global_known_interface_names:
        include_paths.append(KNOWN_INTERFACE_NAMES[name])

    if interface.parent:
        base_class_include = \
            interface_name_to_include_path(interface.name)
    else:
        base_class_include = None

    has_static_member = \
        any([attribute.is_static for attribute in interface.attributes]) or \
        any([operation.is_static for operation in interface.operations]) or \
        constructor['dispatch'] != 'none'

    need_instance_template = \
        any([not attribute['is_static']
             for attribute in attribute_context_list]) or \
        any([not method['is_static']
             for method in method_context_list])

    return {
      'attributes': sort_context_list(attribute_context_list),
      'callbacks': sort_context_list(callback_context_list),
      'class_name': interface.name + 'Class',
      'base_class_include': base_class_include,
      'constants': sort_context_list(constant_context_list),
      'constructor': constructor,
      'dictionaries': dictionaries,
      'enumerations': enumeration_context_list,
      'has_static_member': has_static_member,
      'need_instance_template': need_instance_template,
      'include_paths': sorted(include_paths),
      'interface_name': interface.name,
      'interface_parent': interface.parent,
      'methods': sort_context_list(method_context_list),
      'output_name': interface.name,
      'template_name': 'interface',
    }


######################################################################
#
# Common
#
def cpp_value(value):
    if value == 'NULL':
        return 'nullptr';
    if isinstance(value, bool):
        return str(value).lower()
    return str(value)


def dictionary_name_to_include_path(dictionary_name):
    return FILE_NAME_PREFIX + dictionary_name + '.h'


# Make parameters list without optional parameters.
# Example: foo(T1 a, optional T2 b, optional T3 c)
# Output: [[T1 a], [T1 a, T2 b], [T1 a, T2 b, T3 c]]
def expand_parameters(functions):
    parameters_list = []
    for function in functions:
        parameters = []
        for parameter in function.arguments:
            if parameter.is_optional:
                parameters_list.append(list(parameters))
            parameters.append(function_parameter(parameter))
        parameters_list.append(parameters)
    return parameters_list

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


def interface_name_to_include_path(interface_name):
    if not(interface_name in global_interfaces_info):
        raise Exception('No include path for ' + interface_name)
    interface_info = global_interfaces_info[interface_name]
    if not('include_path' in interface_info):
        raise Exception('No include_path in ' + str(interface_info))
    return interface_info['include_path']


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


# TODO(eval1749) We should not use union_type_string
def union_type_string(type_strings):
    return '|'.join(type_strings)

# Convert lower case cample, lowerCaseCamel, to upper case came, UpperCaseCamel
def upper_camel_case(lower_camel_case):
    return lower_camel_case[0].upper() + lower_camel_case[1:]
