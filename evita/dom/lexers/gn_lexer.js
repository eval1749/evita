// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// See [1] for GN language
// [1] https://chromium.googlesource.com/chromium/src/+/master/tools/gn/docs/language.md
global.GnLexer = (function(keywords) {

  /** @const @type {!Map.<number, !symbol>} */
  const CHARACTERS = (function() {
    const map = new Map();

    function setRange(type, min, max) {
      for (let charCode = min; charCode <= max; ++charCode) {
        map.set(charCode, type);
      }
    }

    map.set(Unicode.LF, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.SPACE, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.TAB, Lexer.WHITESPACE_CHAR);
    map.set(Unicode.QUOTATION_MARK, Lexer.STRING2_CHAR);

    // NameStartChar ::= [a-zA-Z_]
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_CAPITAL_LETTER_A,
             Unicode.LATIN_CAPITAL_LETTER_Z);
    setRange(Lexer.NAMESTART_CHAR,
             Unicode.LATIN_SMALL_LETTER_A,
             Unicode.LATIN_SMALL_LETTER_Z);
    map.set(Unicode.LOW_LINE, Lexer.NAMESTART_CHAR);

    // NameChar ::= NameStart | [0-9]
    setRange(Lexer.NAME_CHAR, Unicode.DIGIT_ZERO, Unicode.DIGIT_NINE);

    return map;
  })();

  //////////////////////////////////////////////////////////////////////
  //
  // GnLexer
  //
  class GnLexer extends global.Lexer {
    /**
     * @param {!TextDocument} document
     */
    constructor(document) {
      super(document, {
        characters: CHARACTERS,
        keywords: Lexer.createKeywords(keywords),
      });
    }
    /**
     * @this {!GnLexer}
     * @param {number} charCode
     */
    feedCharacter(charCode) {
      if (this.state == lexers.State.ZERO && charCode == Unicode.NUMBER_SIGN) {
        this.startToken(lexers.State.LINE_COMMENT);
        return;
      }
      this.updateState(charCode);
    }
  }

  return GnLexer;
})([
  // Conditionals
  'else',
  'false',
  'if',
  'true',

  // Target declarations
  'action',
  'action_foreach',
  'copy',
  'executable',
  'group',
  'loadable_module',
  'shared_library',
  'source_set',
  'static_library',
  'target',

  // Buildfile functions
  'assert',
  'config',
  'declare_args',
  'defined',
  'exec_script',
  'foreach',
  'forward_variables_from',
  'get_label_info',
  'get_path_info',
  'get_target_outputs',
  'getenv',
  'import',
  'print',
  'process_file_template',
  'read_file',
  'rebase_path',
  'set_default_toolchain',
  'set_defaults',
  'set_sources_assignment_filter',
  'template',
  'tool',
  'toolchain',
  'toolchain_args',
  'write_file',

  // //testing/tests.gni
  'test',

  // Built-in predefined variables
  'current_cpu',
  'current_os',
  'current_toolchain',
  'default_toolchain',
  'host_cpu',
  'host_os',
  'python_path',
  'root_build_dir',
  'root_gen_dir',
  'root_out_dir',
  'target_cpu',
  'target_gen_dir',
  'target_os',
  'target_out_dir',

  // Variables you set in targets
  'all_dependent_configs',
  'allow_circular_includes_from',
  'args',
  'asmflags',
  'assert_no_deps',
  'cflags',
  'cflags_c',
  'cflags_cc',
  'cflags_objc',
  'cflags_objcc',
  'check_includes',
  'complete_static_lib',
  'configs',
  'console [boolean]',
  'data',
  'data_deps',
  'defines',
  'depfile',
  'deps',
  'include_dirs',
  'inputs',
  'ldflags',
  'lib_dirs',
  'libs',
  'output_extension',
  'output_name',
  'outputs',
  'precompiled_header',
  'precompiled_source',
  'public',
  'public_configs',
  'public_deps',
  'response_file_contents',
  'script',
  'sources',
  'testonly',
  'visibility',
]);
