{# License text resides in aggregate file, blink_externs.js #}
{% set BLANK_LINE = '' %}
{#////////////////////////////////////////////////////////////////////////
//
// Emit constructor
//
#}
{% macro emit_constructor(name, parent_name, parameters, kind) %}
/**
 * @{{kind}}
{% if parent_name %}
   @extends {{ '{' }}{{parent_name}}{{ '}' }}
{% endif %}
{% for parameter in parameters %}
 * @param {{ '{' }}{{parameter.type}}{{ '}' }} {{parameter.name}}
{% endfor %}
{% if kind == 'constructor' %}
 * @return {undefined}
{% endif %}
 */
{% if namespace %}
{{namespace}}{{name}} = function({{emit_parameters(parameters)}}) {};
{% else %}
function {{name}}({{emit_parameters(parameters)}}) {}
{% endif %}
{% endmacro %}
{#//////////////////////////////////////////////////////////////////////
//
// Example: kind == 'static'
//  /**
//   * @param {!Node} newChild
//   * @param {?Node} refChild
//   * @return {!Node}
//   */
//  Box.classMethod = function(newChild, refChild) {};
//
// Example: kind == 'instance'
//  /**
//   * @param {!Node} newChild
//   * @param {?Node} refChild
//   * @return {!Node}
//   */
//  Box.prototype.insertBefore = function(newChild, refChild) {};
#}
{% macro emit_function(parent_name, name, parameters, return_type, kind) %}
/**
{% for parameter in parameters %}
 * @param {{ '{' }}{{parameter.type}}{{ '}' }} {{parameter.name}}
{% endfor %}
{% if return_type %}
 * @return {{ '{' }}{{return_type}}{{ '}' }}
{% endif %}
 */
{% if kind == 'static' %}
{{parent_name}}.{{name}} = function({{emit_parameters(parameters)}}) {};
{% else %}
{{parent_name}}.prototype.{{name}} = function({{emit_parameters(parameters)}}) {};
{% endif %}

{%- endmacro %}
{% macro emit_parameters(parameters) %}
{{parameters | map(attribute='name') | join(', ')}}
{%- endmacro %}
{% macro emit_parameter_types(parameters) %}
{{parameters | map(attribute='type') | join(', ')}}
{%- endmacro %}
{#//////////////////////////////////////////////////////////////////////
//
// Enumeration
// Example:
//  /**
//   * @enum {string}
//   */
//  var FontFaceLoadStatus = {
//      UNLOADED: 'unloaded',
//      LOADING: 'loading',
//      LOADED: 'loaded',
//      ERROR: 'error' // Note: No trailing comma
//  };
#}
{% for enumeration in enumerations %}
var {{enumeration.name}} = {
{%    for entry in enumeration.entries %}
    {{entry.name}}: "{{entry.value}}"{% if not loop.last %},{% endif %}{{''}}
{%    endfor %}
};
{{ BLANK_LINE }}
{% endfor %}
{#//////////////////////////////////////////////////////////////////////
//
// Callback
// Example:
//  /**
//   * @typedef {!function(type1, type):type}
//   */
//  var MyCallback;
//
#}
{% for callback in callbacks %}
/**
 * @typedef {!function({{emit_parameter_types(callback.parameters)}}){% if callback.type %}:{{callback.type}}{% endif %}}
 */
var {{callback.name}};
{{ BLANK_LINE }}
{% endfor %}
{% for dictionary in dictionaries %}
/**
 * TODO(yosi) Once Closure compiler supports dictionary type, we generate it.
 * Until then, we use |Object|.
 * @typedef {Object}
{%  for member in dictionary.dictionary_members %}
 *    {{member.name}}: {{member.type}}{% if not loop.last %},{% endif %}{{''}}
{%  endfor %}
 */
var {{dictionary.dictionary_name}};
{% endfor %}
{%- for constructor in constructors %}
{{     emit_constructor(constructor.name, constructor.parent_name,
                        constructor.parameters, constructor.kind) }}
{{ BLANK_LINE }}
{%- endfor %}
{#//////////////////////////////////////////////////////////////////////
//
// Static Attributes
//
// Example:
//  /** @const @type {type} */
//  Node.ELEMENT_NODE;
//
#}
{%- for constant in constants %}
/** @const @type {{ '{' }}{{constant.type}}{{ '}' }} */
{{interface_name}}.{{constant.name}} = {{constant.value}};
{{ BLANK_LINE }}
{%- endfor %}
{#//////////////////////////////////////////////////////////////////////
//
// Attributes
//
#}
{%- for attribute in attributes %}
/**
 * @type {{ '{' }}{{attribute.type}}{{ '}' }}
{%- if attribute.is_read_only %}
 * This property is read only.
{%- endif %}
 */
{%- if attribute.is_static %}
{{interface_name}}.{{attribute.name}};
{%- else %}
{{interface_name}}.prototype.{{attribute.name}};
{%- endif %}
{{ BLANK_LINE }}
{%- endfor %}
{#//////////////////////////////////////////////////////////////////////
//
// Methods
//
#}
{%- for method in methods if method.is_static %}
{{  emit_function(interface_name, method.name, method.parameters,
                  method.return_type, 'static') }}
{{ BLANK_LINE }}
{%- endfor %}
{%- for method in methods if not method.is_static and method.name %}
{{     emit_function(interface_name, method.name, method.parameters,
                     method.return_type, 'instance') }}
{{ BLANK_LINE }}
{%- endfor %}
