// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @const @type {!Array.<string>} */
const CSS_PROPERTY_NAMES = [
{% for property in properties %}
  '{{property.name}}',
{% endfor %}
];

/** @const @type {!Map<string, number>} */
const CSS_PROPERTY_NAME_ID_MAP = (function() {
  /** @const @type {!Map<string, number>} */
  const map = new Map();
  let id = 0;
  for (const name in CSS_PROPERTY_NAMES) {
    map.set(name, id);
    ++id;
  }
  return map;
})();

//////////////////////////////////////////////////////////////////////
//
// CSSStyleDeclaration
//
class CSSStyleDeclaration {
  /**
   * @param {!Map<number, string>=} opt_rawStyle
   * @param {!Element=} opt_element
   */
  constructor(opt_rawStyle, opt_element) {
    /** @const @type {Element} */
    this.ownerElement_ = opt_element || null;
    /** @const type {!Map<number, string>} */
    this.rawStyle_ = opt_rawStyle || new Map();
  }

  /** @return {string} */
  get cssText() {
    const properties = [];
    for (const id of this.rawStyle_.keys()) {
      const name = CSSStyleDeclaration.propertyNameOf_(id);
      const value = this.propertyValueOf_(id);
      properties.push(`${name}:${value};`);
    }
    return properties.join(' ');
  }

{% for property in properties %}
  /** @return {string} */
  get {{property.camelCaseName}}() {
    return this.propertyValueOf_({{property.id}});
  }

  /** @param {string} newValue */
  set {{property.camelCaseName}}(newValue) {
    this.setPropertyValue_({{property.id}}, newValue);
  }

{% endfor %}

  /**
   * @param {string} name
   * @return {string}
   */
  getPropertyPriority(name) {
    throw new Error('TODO(eval1749): NYI CSS priority');
  }

  /**
   * @param {string} name
   * @return {string}
   */
  getPropertyValue(name) {
    /** @const @type {number} */
    const id = CSSStyleDeclaration.propertyIdOf_(name);
    if (id < 0)
      return '';
    return this.rawStyle_.get(id) || '';
  }

  /**
   * @private
   * @param {number} id
   * @return {string}
   */
  propertyValueOf_(id) {
    return this.rawStyle_.get(id) || '';
  }

  /**
   * @private
   * @param {number} id
   * @return {string}
   */
  static propertyNameOf_(id) {
    return CSS_PROPERTY_NAMES[id];
  }

  /**
   * @private
   * @param {string} name
   * @return {number}
   */
  static propertyIdOf_(name) {
    return CSS_PROPERTY_NAME_ID_MAP.get(name) || -1;
  }

  /**
   * @param {string} name
   * @return {string}
   */
  removeProperty(name) {
    /** @const @type {number} */
    const id = CSSStyleDeclaration.propertyIdOf_(name);
    if (id < 0)
      return '';
    /** @const @type {string} */
    const value = this.rawStyle_.get(id) || '';
    this.rawStyle_.delete(id);
    return value;
  }

  /**
   * @param {string} name
   * @param {string} newValue
   * @param {string=} newPriority
   */
  setProperty(name, newValue, newPriority = '') {
    this.setPropertyValue(name, newValue);
  }

  /**
   * @param {string} name
   * @param {string} newPriority
   */
  setPropertyPriority(name, newPriority) {
    throw new Error('TODO(eval1749): NYI CSS priority');
  }

  /**
   * @param {string} name
   * @param {string} newValue
   */
  setPropertyValue(name, newValue) {
    /** @const @type {number} */
    const id = CSSStyleDeclaration.propertyIdOf_(name);
    if (id < 0)
      return;
    this.setPropertyValue_(id, newValue);
  }

  /**
   * @private
   * @param {number} id
   * @param {string} newValue
   */
  setPropertyValue_(id, newValue) {
    this.rawStyle_.set(id, newValue);
    if (!this.ownerElement_)
      return;
    NodeHandle.setInlineStyle(this.ownerElement_.handle_, this.rawStyle_);
  }
}
