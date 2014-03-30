// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 */
function DataTransferItemList() {}

/**
 * @type {number}
 */
DataTransferItemList.prototype.length;

/**
 * @param {string|!Uint8Array} data
 * @param {string} mime_type
 */
DataTransferItemList.prototype.add = function(data, mime_type) {};

DataTransferItemList.prototype.clear = function() {};

/**
 * @param {number} index
 * @return {!DataTransferItem}
 */
DataTransferItemList.prototype.get = function(index) {};

/**
 * @param {number} index
 */
DataTransferItemList.prototype.remove = function(index) {};
