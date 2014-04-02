// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @constructor
 * @extends {FormControl}
 */
function TextFieldControl() {}

/**
 * @type{string}
 */
TextFieldControl.prototype.lastChangeEventValue_;

/** @type {!TextFieldSelection} */
TextFieldControl.prototype.selection;

/**
 * @type{string}
 */
TextFieldControl.prototype.value;

/**
 * @type{string}
 * Set |value| property of |TextFieldControl| without dispatching "change"
 * event.
 */
TextFieldControl.prototype.value_;
