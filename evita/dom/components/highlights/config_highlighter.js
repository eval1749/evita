// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const ConfigTokenStateMachine = highlights.ConfigTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

class ConfigPainter extends Painter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new ConfigPainter(document); }
}

class ConfigHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, ConfigPainter.create, new ConfigTokenStateMachine());
  }
}

highlights.ConfigHighlighter = ConfigHighlighter;
// Export |ConfigPainter| for testing.
highlights.ConfigPainter = ConfigPainter;
});

// Override |ConfigLexer| by |ConfigHighlighter|.
// TODO(eval1749): Once we get rid of |ConfigLexer|, we should get rid of this
// override.
global['ConfigLexer'] = highlights.ConfigHighlighter;
