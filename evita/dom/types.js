/**
 * @param {Object} fields Enum type.
 * @param {string} message An error message.
 * @constructor
 */
function EnumError(fields, message) {
  TypeError.call(this, message);
  this.fields = fields;
  // TODO(yosi) This is work around of V8 bug. We should not have
  // |EnumError.message| here.
  this.message = message;
}

EnumError.prototype = Object.create(TypeError.prototype, {
  fields: {
    configurable: true,
    enumerable: true,
    value: [],
    writable: true
  }, // fields
  toString: {
    value: function() {
      return this.message + ' accepts only ' + this.fields + '.';
    },
    configurable: true,
    enumerable: true
  } // toString
});
