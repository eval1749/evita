// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('css.CSSStyleDeclaration');

goog.require('css.Property');
goog.require('css.PropertyParser');
goog.require('unicode');

goog.scope(function() {

/** @constructor */
const Property = css.Property;

/** @constructor */
const CSSStyleObserver = css.CSSStyleObserver;

//////////////////////////////////////////////////////////////////////
//
// CSSStyleDeclaration
//
class CSSStyleDeclaration {
  /**
   * @param {!Map<number, string>=} opt_rawStyle
   * @param {!CSSStyleObserver=} opt_observer
   */
  constructor(opt_rawStyle, opt_observer) {
    /** @const @type {CSSStyleObserver} */
    this.observer_ = opt_observer || null;
    /** @const type {!Map<number, string>} */
    this.rawStyle_ = opt_rawStyle || new Map();
    Object.seal(this);
  }

  /** @return {string} */
  get cssText() {
    const properties = [];
    for (const id of this.rawStyle_.keys()) {
      /** @const @type {string} */
      const name = Property.nameOf(id);
      /** @const @type {string} */
      const value = this.propertyValueOf_(id);
      properties.push(`${name}: ${value};`);
    }
    return properties.join(' ');
  }

  /**
   * @param {string} text
   */
  set cssText(text) {
    [...this.rawStyle_.keys()].forEach(id => this.removePropertyById(id));
    /** @const @type {!css.PropertyParser} */
    const parser = new css.PropertyParser();
    for (const property of parser.parse(text))
      this.setPropertyValue_(property.id, property.value);
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
    const id = Property.idOf(name);
    if (id < 0)
      return '';
    return this.rawStyle_.get(id) || '';
  }

  /** @private */
  notifyChanged() {
    if (!this.observer_)
      return;
    this.observer_.didChangeCSSStyle(this);
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
   * @param {string} name
   * @return {string}
   */
  removeProperty(name) {
    /** @const @type {number} */
    const id = Property.idOf(name);
    if (id < 0)
      return '';
    /** @const @type {string} */
    const value = this.rawStyle_.get(id) || '';
    this.removePropertyById(id);
    return value;
  }

  /** @param {number} id */
  removePropertyById(id) {
    this.rawStyle_.delete(id);
    this.notifyChanged();
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
    const id = Property.idOf(name);
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
    this.notifyChanged();
  }

  /** @override */
  toString() {
    return `CSSStyleDeclaration({${this.cssText}})`;
  }
}

/** @constructor */
css.CSSStyleDeclaration = CSSStyleDeclaration;
});
