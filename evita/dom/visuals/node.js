// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class Node {
  /**
   * @protected
   * @param {Document} document
   * @param {!NodeHandle} handle
   */
  constructor(document, handle) {
    /** @type {!Document} */
    this.document_ = document || /** @type {!Document} */(this);
    /** @const @type {!NodeHandle} */
    this.handle_ = handle;
    /** @type {Node} */
    this.firstChild_ = null;
    /** @type {Node} */
    this.lastChild_ = null;
    /** @type {Node} */
    this.parentNode_ = null;
    /** @type {Node} */
    this.nextSibling_ = null;
    /** @type {Node} */
    this.previousSibling_ = null;

    Node.nodeIdMap_.set(handle.id, this);
  }

  /** @return {Node} */
  get firstChild() { return this.firstChild_; }

  /** @return {Node} */
  get lastChild() { return this.lastChild_; }

  /** @return {Node} */
  get nextSibling() { return this.nextSibling_; }

  /** @return {string} */
  get nodeName() {
    throw new Error('derived class should implement nodeName()')
  }

  /** @return {!Document} */
  get ownerDocument() { return this.document_; }

  /** @return {Node} */
  get parentNode() { return this.parentNode_; }

  /** @return {Node} */
  get previousSibling() { return this.previousSibling_; }

  /**
   * @param {!Node} node
   * @return {!Node}
   */
  appendChild(node) {
    const oldParent = node.parentNode_;
    if (oldParent)
      oldParent.removeChild(node);
    if (node === this || this.isDescendantOf(node))
      throw new Error(`${node} is descendant or self of ${this}`);
    node.parentNode_ = this;
    const oldLastChild = this.lastChild_;
    if (oldLastChild) {
      node.previousSibling_ = oldLastChild;
      oldLastChild.nextSibling_ = node;
    } else {
      this.firstChild_ = node;
    }
    this.lastChild_ = node;
    node.document_ = this.document_;
    NodeHandle.appendChild(this.handle_, node.handle_);
    return node;
  }

  /** @return {boolean} */
  inDocument() {
    for (let runner = this; runner; runner = runner.parentNode_) {
      if (runner === this.document_)
        return true;
    }
    return false;
  }

  /**
   * @param {!Node} node
   * @param {Node} child
   * @return {!Node}
   */
  insertBefore(node, child) {
    if (!child)
      return this.appendChild(node);
    if (child.parentNode_ !== this)
      throw new Error(`${child} isn't child of ${this}`);
    if (node.parentNode_)
      node.parentNode_.removeChild(node);
    const nextSibling = child.nextSibling_;
    const previousSibling = child.previousSibling_;
    if (previousSibling)
      previousSibling.nextSibling_ = node;
    else
      this.firstChild_ = node;
    child.previousSibling_ = node;
    node.document_ = this.document_;
    node.parentNode_ = this;
    node.nextSibling_ = child;;
    node.previousSibling_ = previousSibling;
    NodeHandle.insertBefore(this.handle_, node.handle_, child.handle_);
    return node;
  }

  /**
   * @param {!Node} other
   * @return {boolean}
   */
  isDescendantOf(other) {
    for (let runner = this.parentNode_; runner; runner = runner.parentNode_) {
      if (runner === other)
        return true;
    }
    return false;
  }

  /**
   * @param {number} sequenceId
   * @return {Node}
   */
  static nodeFromId(sequenceId) {
    return Node.nodeIdMap_.get(sequenceId) || null;
  }

  /**
   * @param {!Node} child
   * @return {!Node}
   */
  removeChild(child) {
    if (child.parentNode_ !== this)
      throw new Error(`${child} isn't child of ${this}`);
    const nextSibling = child.nextSibling_;
    const previousSibling = child.previousSibling_;
    if (nextSibling)
      nextSibling.previousSibling_ = previousSibling;
    else
      this.lastChild_ = previousSibling;
    if (previousSibling)
      previousSibling.nextSibling_ = nextSibling;
    else
      this.firstChild_ = nextSibling;
    child.nextSibling_ = null;
    child.previousSibling_ = null;
    child.parentNode_ = null;
    NodeHandle.removeChild(this.handle_, child.handle_);
    return child;
  }

  /**
   * @param {!Node} node
   * @param {!Node} child
   * @return {!Node}
   */
  replaceChild(node, child) {
    if (child.parentNode_ !== this)
      throw new Error(`${child} isn't child of ${this}`);
    if (node.parentNode_)
      node.parentNode_.removeChild(node);
    const nextSibling = child.nextSibling_;
    const previousSibling = child.previousSibling_;
    if (previousSibling)
      previousSibling.nextSibling_ = node;
    else
      this.firstChild_ = node;
    if (nextSibling)
      nextSibling.previousSibling_ = node;
    else
      this.lastChild_ = node;
    child.parentNode_ = null;
    child.nextSibling_ = null;
    child.previousSibling_ = null;
    node.document_ = this.document_;
    node.parentNode_ = this;
    node.nextSibling_ = nextSibling;
    node.previousSibling_ = previousSibling;
    NodeHandle.replaceChild(this.handle_, node.handle_, child.handle_);
    return child;
  }
}

/** @const @type {!Map<number, !Node>} */
Node.nodeIdMap_;

Object.defineProperties(Node, {
  nodeIdMap_: {value: new Map()},
});

Object.defineProperties(Node.prototype, {
  childNodes: {
    get: /** @this {!Node} */ function*() {
      for (let child = this.firstChild; child; child = child.nextSibling_)
        yield child;
    }
  }
});
