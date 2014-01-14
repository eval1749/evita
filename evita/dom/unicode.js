// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  // Well known character codes
  Unicode.CR = 13;
  Unicode.LF = 10;
  Unicode.SPACE = 20;
  Unicode.TAB = 9;
  Unicode.DEL = 0x7F;

  /** @enum{string} */
  Unicode.CATEGORY_MAP = {
    Lu: 'Uppercase_Letter',
    Ll: 'Lowercase_Letter',
    Lt: 'Titlecase_Letter',
    LC: 'Cased_Letter',
    Lm: 'Modifier_Letter',
    Lo: 'Other_Letter',
    L: 'Letter',
    Mn: 'Nonspacing_Mark',
    Mc: 'Spacing_Mark',
    Me: 'Enclosing_Mark',
    M: 'Mark',
    Nd: 'Decimal_Number',
    Nl: 'Letter_Number',
    No: 'Other_Number',
    N: 'Number',
    Pc: 'Connector_Punctuation',
    Pd: 'Dash_Punctuation',
    Ps: 'Open_Punctuation',
    Pe: 'Close_Punctuation',
    Pi: 'Initial_Punctuation',
    Pf: 'Final_Punctuation',
    Po: 'Other_Punctuation',
    P: 'Punctuation',
    Sm: 'Math_Symbol',
    Sc: 'Currency_Symbol',
    So: 'Other_Symbol',
    S: 'Symbol',
    Zs: 'Space_Separator',
    Zl: 'Line_Separator',
    Zp: 'Paragraph_Separator',
    Z: 'Separator',
    Cc: 'Control',
    Cf: 'Format',
    Cs: 'Surrogate',
    Co: 'Private_Use',
    Cn: 'Unassigned',
    C: 'Other'
  };
})();

