// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('base.OrderedSet');
goog.provide('base.OrderedSetNode');

goog.scope(function() {

class OrderedSetNode {
  /**
   * @template T
   * @param {T} data
   */
  constructor(data) {
    /** @type {T} */
    this.data_ = data;
    /** @type {OrderedSetNode<T>} */
    this.left_ = null;
    /** @type {OrderedSetNode<T>} */
    this.parent_ = null;
    /** @type {number} */
    this.priority_ = Math.random();
    /** @type {OrderedSetNode<T>} */
    this.right_ = null;
  }

  /// TODO(eval1749) Closure compiler doesn't recognize T.
  /** $return {T} */
  get data() { return this.data_; }

  /**
   * @return {OrderedSetNode<T>}
   */
  next() {
    let node = this;
    if (node.right_) {
      node = node.right_;
      while (node.left_) {
        node = node.left_;
      }
      return node;
    }
    while (node) {
      const parent = node.parent_;
      if (!parent || parent.left_ === node)
        return parent;
      node = parent;
    }
    return null;
  }

  /**
   * @return {OrderedSetNode<T>}
   */
  previous() {
    let node = this;
    if (node.left_) {
      node = node.left_;
      while (node.right_) {
        node = node.right_;
      }
      return node;
    }
    while (node) {
      const parent = node.parent_;
      if (!parent || parent.right_ === node)
        return parent;
      node = parent;
    }
    return null;
  }

  /** @return {string} */
  toString() { return `OrderedSetNode(${this.data_})`; }
}

/**
 * @template T
 * @param {!base.OrderedSetNode<T>} node
 * @return {OrderedSetNode<T>}
 */
function removeNodeInternal(node) {
  if (!node.left_)
    return node.right_;
  if (!node.right_)
    return node.left_;
  if (node.left_.priority_ < node.right_.priority_) {
    const temp = rotateRight(node);
    temp.right_ = removeNodeInternal(node);
    if (temp.right_)
      temp.right_.parent_ = temp;
    return temp;
  }
  const temp = rotateLeft(node);
  temp.left_ = removeNodeInternal(node);
  if (temp.left_)
    temp.left_.parent_ = temp;
  return temp;
}

/**
 * @template T
 * @param {!base.OrderedSetNode<T>} node
 * @return {!base.OrderedSetNode<T>}
 */
function rotateLeft(node) {
  if (!node.right_)
    throw new Error('Assertion failed: node.right_');
  const temp = node.right_;
  node.right_ = temp.left_;
  if (node.right_)
    node.right_.parent_ = node;
  temp.left_ = node;
  node.parent_ = temp;
  return temp;
}

/**
 * @template T
 * @param {!base.OrderedSetNode<T>} node
 * @return {!base.OrderedSetNode<T>}
 */
function rotateRight(node) {
  if (!node.left_)
    throw new Error('Assertion failed: node.left_');
  const temp = node.left_;
  node.left_ = temp.right_;
  if (node.left_)
    node.left_.parent_ = node;
  temp.right_ = node;
  node.parent_ = temp;
  return temp;
}

class OrderedSet {
  /**
   * @template T
   * @param {!function(!T, !T): boolean} less
   */
  constructor(less) {
    /** @const @type {!function(!T, !T): boolean} */
    this.less_ = less;
    /** @type {OrderedSetNode<T>} */
    this.root_ = null;
    /** @type {number} */
    this.size_ = 0;
  }

  /**
   * @param {T} data
   */
  add(data) {
    this.root_ = this.addImpl(this.root_, new base.OrderedSetNode(data));
    this.root_.parent_ = null;
    ++this.size_;
  }

  /**
   * @private
   * @param {OrderedSetNode<T>} parent
   * @param {!base.OrderedSetNode<T>} node
   * @return {!base.OrderedSetNode<T>}
   */
  addImpl(parent, node) {
    if (!parent)
      return node;
    if (this.less_(node.data_, parent.data_)) {
      const left = this.addImpl(parent.left_, node);
      parent.left_ = left;
      left.parent_ = parent;
      if (left.priority_ > parent.priority_)
        parent = rotateRight(parent);
      return parent;
    }
    const right = this.addImpl(parent.right_, node);
    parent.right_ = right;
    right.parent_ = parent;
    if (right.priority_ > parent.priority_)
      parent = rotateLeft(parent);
    return parent;
  }

  clear() {
    this.root_ = null;
    this.size_ = 0;
  }

  /** @return {boolean} */
  empty() { return this.size_ === 0; }

  /**
   * @param {T} data
   * @return {OrderedSetNode<T>}
   */
  find(data) {
    const node = this.lowerBound(data);
    return node && !this.less_(data, node.data_) ? node : null;
  }

  /**
   * @param {!function(T)} callback
   */
  forEach(callback) {
    for (const data of this.values())
      callback(data);
  }

  /// TODO(eval1749) Closure compiler doesn't recognize T.
  /** $return {T} */
  get maximum() {
    let node = this.root_;
    if (!node)
      throw new Error('OrderedSet is empty.');
    while (node.right_)
      node = node.right_;
    return node.data;
  }

  /// TODO(eval1749) Closure compiler doesn't recognize T.
  /** $return {T} */
  get minimum() {
    let node = this.root_;
    if (!node)
      throw new Error('OrderedSet is empty.');
    while (node.left_)
      node = node.left_;
    return node.data;
  }

  /**  @return {number} */
  get size() { return this.size_; }

  /**
   * @param {T} data
   * @return {OrderedSetNode<T>}
   */
  lowerBound(data) {
    let node = this.root_;
    let found = null;
    while (node) {
      if (this.less_(node.data_, data)) {
        node = node.right_;
      } else {
        found = node;
        node = node.left_;
      }
    }
    return found;
  }

  /**
   * @param {T} data
   * @return {boolean}
   */
  remove(data) {
    const node = this.find(data);
    if (!node)
      return false;
    this.removeNode(node);
    return true;
  }

  /**
   * @public
   * @param {!base.OrderedSetNode<T>} node
   */
  removeNode(node) {
    const parent = node.parent_;
    const child = removeNodeInternal(node);
    if (!parent)
      this.root_ = child;
    else if (parent.left_ === node)
      parent.left_ = child;
    else if (parent.right_ === node)
      parent.right_ = child;
    else
      throw new Error('Tree is broken');
    if (child)
      child.parent_ = parent;
    --this.size_;
  }

  /** @return {!Generator<!base.OrderedSetNode<T>>} */
  * values() {
    let node = this.root_;
    while (node) {
      while (node.left_)
        node = node.left_;
      while (node) {
        yield node.data;
        node = node.next();
      }
    }
  }
}

/** @constructor */
base.OrderedSet = OrderedSet;

/** @constructor */
base.OrderedSetNode = OrderedSetNode;
});
