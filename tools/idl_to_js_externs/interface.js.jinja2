{# License text resides in aggregate file, blink_externs.js #}
{#
 # Emit @constructor block.
 # Example: kind == 'constructor'
 #  /**
 #   * @constructor
 #   * @extends {EventTarget}
 #   * @param {number} width
 #   * @param {number} height
 #   * Note: This is a note text.
 #   */
 #  function Box(width, height) {}
 #
 # Example: kind == 'static'
 #  /**
 #   * @param {!Node} newChild
 #   * @param {?Node} refChild
 #   * @return {!Node}
 #   */
 #  Box.classMethod = function(newChild, refChild) {};
 #
 # Example: kind == 'instance'
 #  /**
 #   * @param {!Node} newChild
 #   * @param {?Node} refChild
 #   * @return {!Node}
 #   */
 #  Box.prototype.insertBefore = function(newChild, refChild) {};
#}
{% macro emit_function(parent_name, name, parameters, return_type, note='', kind='constructor') %}
/**
{% if kind == 'constructor' %}
 * @constructor
{%    if parent_name %}
 * @extends {{ '{' }}{{parent_name}}{{ '}' }}
{%    endif %}
{% endif %}
{% for parameter in parameters %}
 * @param {{ '{' }}{{parameter.type}}{{ '}' }} {{parameter.name}}
{% endfor %}
{% if return_type %}
 * @return {{ '{' }}{{return_type}}{{ '}' }}
{% endif %}
 */
{% if kind == 'constructor' %}
{%  if namespace %}
{{namespace}}{{name}} = function({{emit_parameters(parameters)}}) {};
{%  else %}
function {{name}}({{emit_parameters(parameters)}}) {}
{%  endif %}
{% elif kind == 'static' %}
{{parent_name}}.{{name}} = function({{emit_parameters(parameters)}}) {};
{% else %}
{{parent_name}}.prototype.{{name}} = function({{emit_parameters(parameters)}}) {};
{% endif %}

{%- endmacro %}
{% macro emit_parameters(parameters) %}
{{parameters | map(attribute='name') | join(', ')}}
{%- endmacro %}
{#
 # Enumeration
 # Example:
 #  /**
 #   * @enum {string}
 #   */
 #  var FontFaceLoadStatus = {
 #      UNLOADED: 'unloaded',
 #      LOADING: 'loading',
 #      LOADED: 'loaded',
 #      ERROR: 'error' // Note: No trailing comma
 #  };
 #}
{% for enumeration in enumerations %}
var {{enumeration.name}} = {
{%    for entry in enumeration.entries %}
    {{entry.name}}: "{{entry.value}}"{% if not loop.last %},{% endif %}{{''}}
{%    endfor %}
};

{% endfor %}
{% for callback in callbacks %}
{{    emit_function('', callback.name, callback.parameters) }}
{% endfor %}
{% for dictionary in dictionaries %}
/**
 * TODO(yosi) Once Closure compiler supports dictionary type, we generate it.
 * Until then, we use |Object|.
 * @typedef {Object}
{%  for member in dictionary.members %}
 *    {{member.name}}: {{member.type}}{% if not loop.last %},{% endif %}{{''}}
{%  endfor %}
 */
var {{dictionary.name}};

{% endfor %}
{% for constructor in constructors %}
{{     emit_function(constructor.parent_name, constructor.name,
                     constructor.parameters) }}
{% endfor %}
{% for interface in interfaces %}
{{     emit_function(interface.parent_name, interface.name, [],
                     note='This is derived class. You can\'t create instances.' ) }}
{% endfor %}
{#
 # Static Attributes
 # Example:
 #  /** @const @type {type} */
 #  Node.ELEMENT_NODE;
 #}
{% for constant in constants %}
/** @const @type {{ '{' }}{{constant.type}}{{ '}' }} */
{{interface_name}}.{{constant.name}} = {{constant.value}};

{% endfor %}
{#
 # Attributes
 # Example:
 #  /**
 #   * @type {type}
 #   */
 #  Node.prototype.firstChild;
 #}
{% for attribute in attributes %}
/**
 * @type {{ '{' }}{{attribute.type}}{{ '}' }}
{% if attribute.is_read_only %}
 * This property is read only.
{% endif %}
 */
{% if attribute.is_static %}
{{interface_name}}.{{attribute.name}};
{% else %}
{{interface_name}}.prototype.{{attribute.name}};
{% endif %}

{% endfor %}
{#
 # Methods
 #}
{% for method in methods if method.is_static %}
{{     emit_function(interface_name, method.name, method.parameters,
                     method.return_type, kind='static') }}
{% endfor %}
{% for method in methods if not method.is_static and method.name %}
{{     emit_function(interface_name, method.name, method.parameters,
                     method.return_type, kind='instance') }}
{% endfor %}
