// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'base', function($export) {
  //////////////////////////////////////////////////////////////////////
  //
  // OrderedSetNode
  //
  /** @template T */
  class OrderedSetNode {
    /**
     * @param {T} data
     */
    constructor(data) {
      /** @type {T} */
      this.data_ = data;
      /** @type {base.OrderedSetNode<T>} */
      this.left_ = null;
      /** @type {base.OrderedSetNode<T>} */
      this.parent_ = null;
      /** @type {number} */
      this.priority_ = Math.random();
      /** @type {base.OrderedSetNode<T>} */
      this.right_ = null;
    }

    /** TODO(eval1749) Closure compiler doesn't recognize: $return {T} */
    get data() { return this.data_; }

    /**
     * @return {base.OrderedSetNode<T>}
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
        if (!parent || parent.left_ == node)
          return parent;
        node = parent;
      }
      return null;
    }

    /**
     * @return {base.OrderedSetNode<T>}
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
        if (!parent || parent.right_ == node)
          return parent;
        node = parent;
      }
      return null;
    }

    /** @return {string} */
    toString() {
      return 'OrderedSetNode(' + this.data_.toString() + ')';
    }
  }

  /**
   * @template T
   * @param {!base.OrderedSetNode<T>} node
   * @return {base.OrderedSetNode<T>}
   */
  function removeNode(node) {
    if (!node.left_)
      return node.right_;
    if (!node.right_)
      return node.left_;
    if (node.left_.priority_ < node.right_.priority_) {
      const temp = rotateRight(node);
      temp.right_ = removeNode(node);
      if (temp.right_)
        temp.right_.parent_ = temp;
      return temp;
    }
    const temp = rotateLeft(node);
    temp.left_ = removeNode(node);
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

  //////////////////////////////////////////////////////////////////////
  //
  // OrderedSet
  //
  /** @template T */
  class OrderedSet {
    /**
     * @param {!function(T, T): boolean} less
     */
    constructor(less) {
      /** @const @type {!function(T, T): boolean} */
      this.less_ = less;
      /** @type {base.OrderedSetNode<T>} */
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
     * @param {base.OrderedSetNode<T>} parent
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

    /**
     * @param {T} data
     * @return {base.OrderedSetNode<T>}
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

    /** TODO(eval1749) Closure compiler doesn't recognize: $return {T} */
    get maximum() {
      let node = this.root_;
      if (!node)
        throw new Error('OrderedSet is empty.');
      while (node.right_)
        node = node.right_;
      return node.data;
    }

    /** TODO(eval1749) Closure compiler doesn't recognize: $return {T} */
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
     * @return {base.OrderedSetNode<T>}
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
      const parent = node.parent_;
      const child = removeNode(node);
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

    /** @return {!Generator.<!base.OrderedSetNode<T>>} */
    *values() {
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

  $export({OrderedSet, OrderedSetNode});
});
