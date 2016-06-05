// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var suggestions;

/** @interface */
suggestions.Provider = function() {};

/** @return {boolean} */
suggestions.Provider.prototype.isReady = function() {};

/** @return {string} */
suggestions.Provider.prototype.next = function() {};
