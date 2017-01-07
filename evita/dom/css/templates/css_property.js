// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.Property');

goog.scope(function() {

/** @const @type {!Array<string>} */
const CSS_PROPERTY_NAMES = [
  "invalid",
{% for property in properties %}
  {{property.text}},
{% endfor %}
];

/** @const @type {!Map<string, number>} */
const CSS_PROPERTY_NAME_ID_MAP = (function() {
  /** @const @type {!Map<string, number>} */
  const map = new Map();
  let id = 0;
  for (const name of CSS_PROPERTY_NAMES) {
    map.set(name, id);
    ++id;
  }
  return map;
})();

class Property {
  /**
   * @public
   * @param {string} name
   * @param {string} value
   */
  constructor(name, value) {
    /** @const @type {number} */
    this.id_ = Property.idOf(name);
    /** @const @type {string} */
    this.name_ = name;
    /** @const @type {string} */
    this.value_ = value;
  }

  /** @public @return {number} */
  get id() { return this.id_; }

  /** @public @return {string} */
  get name() { return this.name_; }

  /** @public @return {string} */
  get value() { return this.value_; }

  /**
   * @public
   * @param {number} id
   * @return {string}
   */
  static nameOf(id) {
    return CSS_PROPERTY_NAMES[id];
  }

  /**
   * @public
   * @param {string} name
   * @return {number}
   */
  static idOf(name) {
    return CSS_PROPERTY_NAME_ID_MAP.get(name) || -1;
  }

  /** @override */
  toString() { return `Property('${this.name}: ${this.value_}')`; }
}

/** @constructor */
css.Property = Property;
});
