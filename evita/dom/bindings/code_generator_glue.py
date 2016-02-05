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

# We'll include "gc/member.h" when one of members of dictionary is collectable.
global_has_gc_member = False

# We'll include "ginx/nullable.h" when return value is nullable.
global_has_nullable = False
global_referenced_dictionary_names = set()
global_referenced_interface_names = set()
global_definitions = {}
global_interfaces_info = {}

FILE_NAME_PREFIX = 'ginx_'

JS_INTERFACE_NAMES = {
    'ArrayBufferView': 'gin/array_buffer.h',
    'LineAndColumn': 'evita/text/buffer.h',
    'Point': 'evita/dom/windows/point.h',
    'Rect': 'evita/dom/windows/rect.h',
}
global_js_interface_names = set()


def can_fast_return_of(glue_type):
    # Note: to_v8_str() returns 'auto'.
    return_str = glue_type.return_str()
    return return_str in ['bool', 'int', 'double', 'float']


def should_be_callback(idl_type):
    if idl_type.is_callback_function or idl_type.is_callback_interface:
        return True
    assert not idl_type.name.endswith('Callback')
    return False

######################################################################
#
# GlueType
#


class GlueType(object):

    def __init__(self, idl_type, cc_name, is_by_value=True,
                 is_collectable=False, is_pointer=False, is_struct=False):
        self.cc_name = cc_name
        self.idl_type = idl_type
        self.is_by_value = is_by_value
        self.is_collectable = is_collectable
        self.is_nullable = idl_type.is_nullable
        self.is_pointer = is_pointer
        self.is_struct = is_struct
        if self.idl_type.is_array or self.idl_type.is_sequence or \
                idl_type.is_array_or_sequence_type:
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
            return 'gc::Member<%s>' % self.cc_name
        if self.is_pointer:
            return self.cc_name + '*'
        return self.cc_name

    def display_str(self):
        return str(self.idl_type)

    # Used for variable declaration of output parameter of
    # |gin::ConvertFromV8|.
    def from_v8_str(self):
        if self.idl_type.is_union_type:
            members = [member for member in self.idl_type.idl_types()]
            return 'ginx::Either<' + \
                ', '.join(to_glue_type(member_type).from_v8_str()
                          for member_type in members[1:]) + '>'
        if self.element_typestr:
            return 'std::vector<%s>' % self.element_typestr
        if self.is_collectable:
            if self.idl_type.is_nullable:
                global global_has_nullable
                global_has_nullable = True
                return 'ginx::Nullable<%s>' % self.cc_name
            return self.cc_name + '* /* from_v8 is_collectable */'
        if self.is_pointer:
            return self.cc_name + '* /* from_v8 is_pointer */'
        return self.cc_name

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
            return self.cc_name + '* /* return_str is_collectable */'
        if self.is_pointer:
            return self.cc_name + '* /* return_str is_ponter */'
        return self.cc_name

    def to_v8_str(self):
        if self.cc_name == 'void':
            return self.cc_name
        if self.is_collectable and self.idl_type.is_nullable:
            global global_has_nullable
            global_has_nullable = True
            return 'ginx::Nullable<%s>' % self.cc_name
        return 'auto'


class CppType(object):

    def __init__(self, cc_name, is_by_value=True, is_collectable=False,
                 is_pointer=False, is_struct=False):
        self.cc_name = cc_name
        self.is_by_value = is_by_value
        self.is_collectable = is_collectable
        self.is_pointer = is_pointer
        self.is_struct = is_struct

    def __str__(self):
        return 'CppType(' + self.cc_name + \
            (' by_value' if self.is_by_value else '') + \
            (' collectable' if self.is_collectable else '') + \
            (' pointer' if self.is_pointer else '') + \
            (' struct' if self.is_pointer else '') + ')'


IDL_TO_CPP_TYPE_MAP = {
    # TODO(eval1749): We should have "SwitchValue.idl".
    'SwitchValue': CppType('domapi::SwitchValue', is_by_value=False),
    # TODO(eval1749): We should have "TabData.idl".
    'TabData': CppType('domapi::TabData', is_by_value=False),

    # Glue specific types
    'Alter': CppType('Alter'),
    'ArrayBufferView': CppType('gin::ArrayBufferView', is_by_value=False),
    'DataTransferData': CppType('DataTransferData', is_pointer=True),
    'EventListener': CppType('v8::Local<v8::Object>'),
    'LineAndColumn': CppType('text::LineAndColumn'),
    # For NodeHandle.getInlineStyle
    'Map': CppType('v8::Local<v8::Map>'),
    # For Window.prototype.compute_
    'Point': CppType('domapi::FloatPoint'),
    # For Window.prototype.compute_
    'Rect': CppType('domapi::FloatRect'),
    # For Editor.runScript
    'RunScriptResult': CppType('v8::Local<v8::Object>'),
    # For Editor.localizeText
    'StringDict': CppType('v8::Local<v8::Object>'),
    'TextStyle': CppType('v8::Local<v8::Object>'),
    'TextOffset': CppType('text::Offset'),
    'Unit': CppType('Unit'),

    # V8 types
    'Function': CppType('v8::Local<v8::Function>'),
    'Object': CppType('v8::Local<v8::Object>'),
    'Promise': CppType('v8::Local<v8::Promise>'),

    # IDL types
    'DOMString': CppType('base::string16', is_by_value=False),
    # DOMTimeStamp is defined in "core/dom/CommonDefinitions.idl".
    'DOMHighResTimeStamp': CppType('double'),
    'DOMTimeStamp': CppType('TimeStamp'),
    'any': CppType('v8::Local<v8::Value>'),
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
    if idl_type.is_array or idl_type.is_sequence_type or \
            idl_type.is_array_or_sequence_type:
        return GlueType(idl_type, idl_type.element_type.base_type)

    if idl_type.is_union_type:
        return GlueType(idl_type, idl_type.name)

    type_name = idl_type.base_type

    if type_name is None:
        raise 'to_glue_type(' + str(idl_type) + ')'

    if type_name in IDL_TO_CPP_TYPE_MAP:
        if type_name in JS_INTERFACE_NAMES:
            global_js_interface_names.add(type_name)
        cpp_type = IDL_TO_CPP_TYPE_MAP[type_name]
        return GlueType(idl_type, cpp_type.cc_name,
                        is_by_value=cpp_type.is_by_value,
                        is_pointer=cpp_type.is_pointer)

    if idl_type.is_dictionary:
        global_referenced_dictionary_names.add(type_name)
        return GlueType(idl_type, type_name, is_struct=True)

    if should_be_callback(idl_type):
        return GlueType(idl_type, 'v8::Local<v8::Function>', is_struct=True)

    if idl_type.is_interface_type:
        assert idl_type.is_interface_type, idl_type
        assert not idl_type.name.endswith(
            'Callback'), 'Should be callback ' + idl_type.name
        assert not idl_type.name.endswith(
            'Init'), 'Should be dictionary ' + idl_type.name
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
    IdlType.set_implemented_as_interfaces(
        interfaces_info['implemented_as_interfaces'])
    IdlType.set_garbage_collected_types(
        interfaces_info['garbage_collected_interfaces'])
    IdlType.set_will_be_garbage_collected_types(
        interfaces_info['will_be_garbage_collected_interfaces'])


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

        # for interface_info in interfaces_info.values():
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
                'contents': self.generate_contents(context, extension),
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
            [build_interface_context(interface)
             for interface in definitions.interfaces.values()]
        ]))


######################################################################
#
# Context classes
#
class BaseContext(object):
    """BaseContext"""

    def __init__(self, root_context, name):
        self._cc_name = None
        self._attributes = None
        self._is_static = None
        self._name = name
        self._root_context = root_context

    @property
    def cc_name(self):
        if not self._cc_name:
            raise Exception('%s does not have cc_name.' % self.name)
        return self._cc_name

    @property
    def root_context(self):
        return self._root_context

    @property
    def is_javascript(self):
        return self.cc_name == 'JavaScript'

    @property
    def is_static(self):
        if self._is_static == None:
            raise Exception('%s does not have is_static.' % self.name)
        return self._is_static

    @property
    def name(self):
        return self._name

    def attribute_of(self, name, empty_value):
        if self._attributes == None:
            raise Exception('Extended attributes is not initialized.' %
                            self.name)
        if not(name in self._attributes):
            return None
        value = self._attributes[name]
        if value == None:
            return empty_value
        return value

    def add_include(self, include_path):
        self._root_context.add_include(include_path)

    def set_cc_name(self, cc_name):
        if self._cc_name != None and self._cc_name != cc_name:
            raise Exception('%s already has cc_name %s.' %
                            (self.name, self._cc_name))
        self._cc_name = cc_name

    def set_attributes(self, attributes):
        self._attributes = attributes

    def set_is_static(self, is_static):
        if self._is_static != None and self._is_static != is_static:
            raise Exception('%s already has is_static.' % self.name)
        self._is_static = is_static


######################################################################
#
# AttributeContext
#
class AttributeContext(BaseContext):
    """AttributeContext represents an attribute in interface"""

    def __init__(self, root_context, name):
        super(AttributeContext, self).__init__(root_context, name)

    def build(self, attribute):
        self.set_attributes(attribute.extended_attributes)
        cc_name = cc_property_name_of(attribute)
        self.set_cc_name(cc_name)
        self.set_is_static(attribute.is_static)
        if self.is_javascript:
            return {'cc_name': cc_name}
        glue_type = to_glue_type(attribute.idl_type)
        is_raises_exception = self.attribute_of('RaisesException', '*')
        return {
            'can_fast_return': can_fast_return_of(glue_type),
            'cc_name': cc_name,
            'display_type': glue_type.display_str(),
            'from_v8_type': glue_type.from_v8_str(),
            'getter_raises_exception':
                is_raises_exception == '*' or is_raises_exception == 'Getter',
            'is_read_only': attribute.is_read_only,
            'is_static': attribute.is_static,
            'name': attribute.name,
            'setter_raises_exception':
                is_raises_exception == '*' or is_raises_exception == 'Setter',
            'to_v8_type': glue_type.to_v8_str(),
        }


######################################################################
#
# FunctionContext
#
class FunctionContext(BaseContext):

    def __init__(self, root_context, name):
        super(FunctionContext, self).__init__(root_context, name)
        self._signatures = []
        self._use_call_with = False
        self._use_exception_state = False

    @property
    def signatures(self):
        return self._signatures

    @property
    def use_exception_state(self):
        return self._use_exception_state

    # Make signatures without optional parameters.
    # Example: foo(T1 a, optional T2 b, optional T3 c)
    # Output: [[T1 a], [T1 a, T2 b], [T1 a, T2 b, T3 c]]
    def build_signatures(self, cc_name, model, attributes):
        is_raises_exception = self.raises_exception(model, attributes)
        if is_raises_exception:
            self._use_exception_state = True

        call_with_list = []
        if 'CallWith' in attributes:
            call_with_list = attributes['CallWith'].split('|')
            self._use_call_with = True

        to_v8_type = self.return_type_of(model)
        for parameters in make_parameters_list(model):
            self._signatures.append({
                'call_with_list': call_with_list,
                'cc_name': cc_name,
                'is_raises_exception': is_raises_exception,
                'is_static': self.is_static,
                'name': model.name,
                'parameters': parameters,
                'to_v8_type': to_v8_type,
            })

    def finish(self):
        if self._use_call_with:
            self.add_include('evita/ginx/runner.h')
        signatures = self._signatures
        if not signatures:
            return {'dispatch': 'none'}
        max_arity = max([len(signature['parameters'])
                         for signature in signatures])
        min_arity = min([len(signature['parameters'])
                         for signature in signatures])
        context = {
            'cc_name': signatures[0]['cc_name'],
            'max_arity': max_arity,
            'min_arity': min_arity,
            'name': self.name,
            'use_call_with': self._use_call_with,
            'use_exception_state': self._use_exception_state,
        }
        if len(signatures) == 1:
            context['dispatch'] = 'single'
            context['signature'] = signatures[0]
            return context
        arity_set = set([len(signature['parameters'])
                         for signature in signatures])
        if len(arity_set) == len(signatures):
            context['dispatch'] = 'arity'
            context['signatures'] = signatures
            return context
        raise Exception('NYI: type based dispatch')

    def raises_exception(self, model, attributes):
        if not('RaisesException' in attributes):
            return False
        value = attributes['RaisesException']
        if value == None:
            return True
        if model.is_constructor:
            return value == 'Constructor'
        return False

    def return_type_of(self, model):
        """Returns C++ function return type to pass V8 of specified |model|."""
        if model.idl_type == None:
            return None
        glue_type = to_glue_type(model.idl_type)
        return glue_type.to_v8_str()


######################################################################
#
# ConstructorContext
#
# IdlConstructor is derived class of IdlOperation.
# IdlConstructor.name = 'Constructor' | 'NamedConstructor'
#
class ConstructorContext(FunctionContext):
    """ConstructorContext represents constructor in interface"""

    def __init__(self, root_context):
        super(ConstructorContext, self).__init__(root_context,
                                                 root_context.name)
        self.set_is_static(True)

    def build(self, interface):
        # Using |new| operator
        for constructor in interface.constructors:
            cc_name = 'new ' + interface.name
            self.build_signatures(cc_name, constructor,
                                  interface.extended_attributes)

        # Custom constructor. It is used for validating parameters and
        # throw JavaScript exception for an invalid parameter.
        for constructor in interface.custom_constructors:
            cc_name = interface.name + '::New' + interface.name
            self.build_signatures(cc_name, constructor,
                                  interface.extended_attributes)
        return self.finish()


######################################################################
#
# RootContext
#
class RootContext(BaseContext):
    """RootContext represents interface or dictionary."""

    def __init__(self, root_model):
        super(RootContext, self).__init__(self, root_model.name)
        self._include_paths = set()
        self._root_model = root_model
        self.set_cc_name(cc_function_name_of(root_model))

    @property
    def include_paths(self):
        return sorted([path for path in self._include_paths])

    def add_include(self, include_path):
        self._include_paths.add(include_path)


######################################################################
#
# InterfaceContext
#
class InterfaceContext(RootContext):
    """InterfaceContext represents context of interface"""

    def __init__(self, root_model):
        super(InterfaceContext, self).__init__(root_model)
        self.add_include('base/logging.h')
        self.add_include('evita/dom/bindings/exception_state.h')
        self.add_include('evita/dom/converter.h')
        self.add_include('evita/dom/script_host.h')
        self.add_include('evita/ginx/constructor_template.h')
        self.add_include('evita/ginx/function_template_builder.h')

    def finish(self):
        for name in global_referenced_interface_names:
            if not 'include_path' in global_interfaces_info[name]:
                continue
            self.add_include(interface_name_to_include_path(name))
        for name in global_referenced_dictionary_names:
            self.add_include(dictionary_name_to_include_path(name))
        if global_has_gc_member:
            self.add_include('evita/gc/member.h')
        if global_has_nullable:
            self.add_include('evita/ginx/nullable.h')
        for name in global_js_interface_names:
            self.add_include(JS_INTERFACE_NAMES[name])


######################################################################
#
# MethodGroupContext
#
class MethodGroupContext(FunctionContext):
    """MethodGroupContext represents method group in interface"""

    def __init__(self, root_context, name):
        super(MethodGroupContext, self).__init__(root_context, name)

    def build(self, methods):
        for method in methods:
            cc_name = cc_function_name_of(method)
            self.set_cc_name(cc_name)
            self.set_is_static(method.is_static)
            if self.is_javascript:
                return {
                    'cc_name': cc_name,
                    'is_static': method.is_static,
                    'name': method.name,
                }
            self.build_signatures(cc_name, method, method.extended_attributes)
        assert(not self.is_javascript)
        context = self.finish()
        context['is_static'] = self.is_static
        return context


######################################################################
#
# Dictionary
#
def dictionary_member_context(member):
    if 'ImplementedAs' in member.extended_attributes:
        cc_name = member.extended_attributes['ImplementedAs']
    else:
        cc_name = underscore(member.name)

    if member.idl_type.is_nullable:
        global global_has_nullable
        global_has_nullable = True

    glue_type = to_glue_type(member.idl_type)
    return {
        'cc_name': cc_name,
        'declare_type': glue_type.declare_str(),
        'default_value': cpp_value(member.default_value),
        'display_type': glue_type.display_str(),
        'from_v8_type': glue_type.from_v8_str(),
        'has_default_value': member.default_value is not None,
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
                           filter(lambda name: 'include_path' in
                                  global_interfaces_info[name],
                                  global_referenced_interface_names)) + \
        map(dictionary_name_to_include_path,
            global_referenced_dictionary_names)
    cc_include_paths.append('evita/dom/converter.h')
    if global_has_nullable:
        cc_include_paths.append('evita/ginx/nullable.h')
    cc_include_paths.append('evita/dom/v8_strings.h')

    for name in global_js_interface_names:
        class_references.append(name)
        cc_include_paths.append(JS_INTERFACE_NAMES[name])

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


def enumeration_context(enumeration):
    # FIXME: Handle empty string value. We don't have way to express empty
    # string as Glue externs as of March 2014.
    return {
        'name': enumeration.name,
        'entries': [{'name': value.upper(), 'value': value}
                    for value in sorted(enumeration.values) if value],
    }


def fix_include_path(path_in):
    path = path_in.replace('CSS', 'Css')
    return os.path.join(os.path.dirname(path).replace('../', ''),
                        underscore(os.path.basename(path))).replace('\\', '/')


def function_parameter(parameter):
    glue_type = to_glue_type(parameter.idl_type)
    return {
        'cc_name': underscore(parameter.name),
        'display_type': glue_type.display_str(),
        'from_v8_type': glue_type.from_v8_str(),
    }


def build_interface_context(interface):
    interface_context = InterfaceContext(interface)

    if interface_context.is_javascript:
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

    enumeration_context_list = [
        enumeration_context(enumeration)
        for enumeration in global_definitions.enumerations.values()
    ]

    attribute_context_list = filter(
        lambda context: context['cc_name'] != 'JavaScript',
        [AttributeContext(interface_context, attribute.name).build(attribute)
         for attribute in interface.attributes])

    constant_context_list = [
        constant_context(constant)
        for constant in interface.constants
    ]

    constructor = ConstructorContext(interface_context).build(interface)

    dictionaries = [
        {'name': dictionary.name}
        for dictionary in global_definitions.dictionaries.values()
    ]

    method_context_list = filter(
        lambda context: context['cc_name'] != 'JavaScript',
        [MethodGroupContext(interface_context, name).build(list(operations))
         for name, operations in
         groupby(interface.operations, lambda operation: operation.name)])

    global_referenced_interface_names.add(interface.name)
    interface_context.finish()

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
        'include_paths': interface_context.include_paths,
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
        return 'nullptr'
    if isinstance(value, bool):
        return str(value).lower()
    return str(value)


def dictionary_name_to_include_path(dictionary_name):
    return FILE_NAME_PREFIX + dictionary_name + '.h'


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


def interface_name_to_include_path(interface_name):
    if not(interface_name in global_interfaces_info):
        raise Exception('No include path for ' + interface_name)
    interface_info = global_interfaces_info[interface_name]
    if not('include_path' in interface_info):
        raise Exception('No include_path in ' + str(interface_info))
    return interface_info['include_path']


def make_parameters_list(function):
    parameters_list = []
    parameters = []
    for parameter in function.arguments:
        if parameter.is_optional:
            parameters_list.append(list(parameters))
        parameters.append(function_parameter(parameter))
    parameters_list.append(parameters)
    return parameters_list


def sort_context_list(context_list):
    return sorted(context_list, key=lambda context: context['name'])


def cc_function_name_of(model):
    """Returns C++ function name for implementing |model| in C++,
       e.g. CamelCase"""
    if 'ImplementedAs' in model.extended_attributes:
        return model.extended_attributes['ImplementedAs']
    return upper_camel_case(model.name)


def cc_property_name_of(model):
    """Returns C++ proeprty name for implementing |model| in C++."""
    if 'ImplementedAs' in model.extended_attributes:
        return model.extended_attributes['ImplementedAs']
    return underscore(model.name)


def underscore(text):
    result = ''
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


# TODO(eval1749): We should not use union_type_string
def union_type_string(type_strings):
    return '|'.join(type_strings)

# Convert lower case camel, lowerCaseCamel, to upper case camel, UpperCaseCamel


def upper_camel_case(lower_camel_case):
    return lower_camel_case[0].upper() + lower_camel_case[1:]
