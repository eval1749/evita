// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.OrderedSet = (function() {
  /**
   * _constructor
   * @template T
   * @param {!function(T, T): boolean} less
   */
  function OrderedSet(less) {
    this.less_ = less;
    this.root_ = null;
    this.size_ = 0;
  }

  /**
   * @template T
   * @param {T} data
   */
  global.OrderedSetNode = function(data) {
    this.data_ = data;
    this.left_ = null;
    this.parent_ = null;
    this.priority_ = Math.random();
    this.right_ = null;
  }

  /**
   * @template T
   * @type {T}
   */
  OrderedSetNode.prototype.data_;

  /**
   * @template T
   * @type {?OrderedSetNode.<T>}
   */
  OrderedSetNode.prototype.left_;

  /**
   * @private
   * @template T
   * @type {?OrderedSetNode.<T>}
   */
  OrderedSetNode.prototype.parent_;

  /**
   * @private
   * @type {number}
   */
  OrderedSetNode.prototype.priority_;

  /**
   * @private
   * @template T
   * @type {?OrderedSetNode.<T>}
   */
  OrderedSetNode.prototype.right_;

  /**
   * @private
   * @template T
   * @this {!OrderedSetNode.<T>}
   * @return {?OrderedSetNode.<T>}
   */
  OrderedSetNode.prototype.next;

  /**
   * @private
   * @template T
   * @this {!OrderedSetNode.<T>}
   * @return {?OrderedSetNode.<T>}
   */
  OrderedSetNode.prototype.previous;

  /**
   * @template T
   * @this {!OrderedSetNode.<T>}
   * @return {?OrderedSetNode.<T>}
   */
  function nextNode() {
    var node = this;
    if (node.right_) {
      node = node.right_;
      while (node.left_) {
        node = node.left_;
      }
      return node;
    }
    while (node) {
      var parent = node.parent_;
      if (!parent || parent.left_ == node)
        return parent;
      node = parent;
    }
    return null;
  }

  /**
   * @template T
   * @this {!OrderedSetNode.<T>}
   * @return {?OrderedSetNode.<T>}
   */

  function previousNode() {
    var node = this;
    if (node.left_) {
      node = node.left_;
      while (node.right_) {
        node = node.right_;
      }
      return node;
    }
    while (node) {
      var parent = node.parent_;
      if (!parent || parent.right_ == node)
        return parent;
      node = parent;
    }
    return null;
  }

  /**
   * @template T
   * @param {!OrderedSetNode.<T>} node
   * @return {?OrderedSetNode.<T>}
   */
  function removeNode(node) {
    if (!node.left_)
      return node.right_;
    if (!node.right_)
      return node.left_;
    if (node.left_.priority_ < node.right_.priority_) {
      var temp = rotateRight(node);
      temp.right_ = removeNode(node);
      if (temp.right_)
        temp.right_.parent_ = temp;
      return temp;
    }
    var temp = rotateLeft(node);
    temp.left_ = removeNode(node);
    if (temp.left_)
      temp.left_.parent_ = temp;
    return temp;
  }

  /**
   * @template T
   * @param {!OrderedSetNode.<T>} node
   * @return {!OrderedSetNode.<T>}
   */
  function rotateLeft(node) {
    if (!node.right_)
      throw new Error('Assertion failed: node.right_');
    var temp = node.right_;
    node.right_ = temp.left_;
    if (node.right_)
      node.right_.parent_ = node;
    temp.left_ = node;
    node.parent_ = temp;
    return temp;
  }

  /**
   * @template T
   * @param {!OrderedSetNode.<T>} node
   * @return {!OrderedSetNode.<T>}
   */
  function rotateRight(node) {
    if (!node.left_)
      throw new Error('Assertion failed: node.left_');
    var temp = node.left_;
    node.left_ = temp.right_;
    if (node.left_)
      node.left_.parent_ = node;
    temp.right_ = node;
    node.parent_ = temp;
    return temp;
  }

  OrderedSetNode.prototype = Object.create(
      /** @type {!Object} */(Object.prototype), {
    constructor: {value: OrderedSetNode},
    data: {get:
      /** @this {!OrderedSetNode} @return {*} */
      function() { return this.data_; }
    },
    next: {value: nextNode},
    previous: {value: previousNode},
    toString: {value:
      /** @return {string} */
      function() {
        return 'OrderedSetNode(' + this.data_.toString() + ')';
      }
    },
  });

  /**
   * @private
   * @template T
   * @type {OrderedSetNode.<T>}
   */
  OrderedSet.prototype.root_;

  /**
   * @this {OrderedSet}
   * @template T
   * @param {T} data
   */
  function add(data) {
    this.root_ = addImpl(this.less_, this.root_, new OrderedSetNode(data));
    this.root_.parent_ = null;
    ++this.size_;
  }

  /**
   * @template T
   * @param {!function(T, T): boolean} less
   * @param {?OrderedSetNode.<T>} parent
   * @param {!OrderedSetNode.<T>} node
   * @return {!OrderedSetNode.<T>}
   */
  function addImpl(less, parent, node) {
    if (!parent)
      return node;
    if (less(node.data_, parent.data_)) {
      var left = addImpl(less, parent.left_, node);
      parent.left_ = left;
      left.parent_ = parent;
      if (left.priority_ > parent.priority_)
        parent = rotateRight(parent);
      return parent;
    }
    var right = addImpl(less, parent.right_, node);
    parent.right_ = right;
    right.parent_ = parent;
    if (right.priority_ > parent.priority_)
      parent = rotateLeft(parent);
    return parent;
  }

  /**
   * @template T
   * @this {OrderedSet}
   * @param {T} data
   * @return {?OrderedSetNode.<T>}
   */
  function find(data) {
    var node = lowerBoundImpl(this.less_, this.root_, data);
    return node && !this.less_(data, node.data_) ? node : null;
  }

  /**
   * @template T
   * @this {OrderedSet}
   * @param {!function(T)} callback
   */
  function forEach(callback) {
    if (!this.root_)
      return;
    var node = this.root_;
    while (node.left_) {
      node = node.left_;
    }
    while (node) {
      callback(node.data_);
      node = node.next();
    }
  }

  /**
   * @template T
   * @this {OrderedSet}
   * @param {T} data
   * @return {?OrderedSetNode.<T>}
   */
  function lowerBound(data) {
    return lowerBoundImpl(this.less_, this.root_, data);
  }

  /**
   * @template T
   * @param {!function(T, T): boolean} less
   * @param {?OrderedSetNode.<T>} node
   * @param {T} data
   * @return {?OrderedSetNode.<T>}
   */
  function lowerBoundImpl(less, node, data) {
    var found = null;
    while (node) {
      if (less(node.data_, data)) {
        node = node.right_;
      } else {
        found = node;
        node = node.left_;
      }
    }
    return found;
  }

  /**
   * @template T
   * @this {OrderedSet}
   * @param {T} data
   * @return {boolean}
   */
  function remove(data) {
    var node = this.find(data);
    if (!node)
      return false;
    var parent = node.parent_;
    var child = removeNode(node);
    if (!parent)
      this.root_ = child;
    else if (parent.left_ == node)
      parent.left_ = child;
    else if (parent.right_ == node)
      parent.right_ = child;
    else
      throw new Error('Tree is broken');
    if (child)
      child.parent_ = parent;
    --this.size_;
    return true;
  }

  OrderedSet.prototype = Object.create(
      /** @type {!Object} */(Object.prototype), {
    Node: {value: OrderedSetNode},
    add: {value: add},
    constructor: {value: OrderedSet},
    find: {value: find},
    forEach: {value: forEach},
    less_: {writable: true},
    lowerBound: {value: lowerBound},
    remove: {value: remove},
    root_: {value: null, writable: true},
    size_: {value: 0, writable: true},
    size: {get:
      /** @this {!OrderedSet} @return {number} */
      function() { return this.size_; }
    }
  });
  return OrderedSet;
})();
