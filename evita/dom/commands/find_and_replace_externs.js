// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @type {{
 *    find: function(!Window, string, !FindAndReplaceOptions): ?Editor.RegExp,
 *    replaceOne: function(!Window, string, string, !FindAndReplaceOptions),
 *    replaceAll: function(!Window, string, string, !FindAndReplaceOptions)
 * }}
 */
var FindAndReplace;

/**
 * @typedef {{
 *    backward: boolean,
 *    casePreserveReplacement: boolean,
 *    ignoreCase: boolean,
 *    matchWholeWord: boolean,
 *    useRegExp: boolean
 *  }}
 */
var FindAndReplaceOptions;
