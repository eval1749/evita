{# License text resides in aggregate file, blink_externs.js #}
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
/**
 * TODO(yosi) Once Closure compiler supports dictionary type, we generate it.
 * Until then, we use |Object|.
 * @typedef {Object}
{%  for member in dictionary_members %}
 *    {{member.name}}: {{member.type}}{% if not loop.last %},{% endif %}{{''}}
{%  endfor %}
 */
{% if namespace %}
{{namespace}}{{dictionary_name}};
{% else %}
var {{dictionary_name}};
{% endif %}
