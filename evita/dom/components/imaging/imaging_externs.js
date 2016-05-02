// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('imaging');

/** @constructor */
imaging.IconUtil = function() {};

/**
 * @public
 * @param {string} fileName
 * @param {string} iconGroupName
 * @param {number=} opt_iconSize
 * @return {!Promise<ImageData>}
 */
imaging.IconUtil.loadIcon = function(fileName, iconGroupName, opt_iconSize) {};

/**
 * @public
 * @param {string} fileName
 * @param {number} iconGroupIndex
 * @param {number=} opt_iconSize
 * @return {!Promise<ImageData>}
 */
imaging.IconUtil.loadIconByIndex = function(
    fileName, iconGroupIndex, opt_iconSize) {};

/**
 * @public
 * @param {string} extension
 * @param {number=} opt_iconSize
 * @return {!Promise<ImageData>}
 */
imaging.IconUtil.loadIconForExtension = function(extension, opt_iconSize) {};
