// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
function DataTransferItem() {}

/**
 * @type {string}
 */
DataTransferItem.prototype.kind;

/**
 * @type {string}
 */
DataTransferItem.prototype.type;

/**
 * @return {!Uint8Array}
 */
DataTransferItem.prototype.getAsBlob = function() {};

/**
 * @return {string}
 */
DataTransferItem.prototype.getAsString = function() {};
