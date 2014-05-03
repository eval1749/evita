// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   * @extends {Lexer}
   * @param {!Document} document
   */
  function PlainTextLexer(document) {
  }
  PlainTextLexer.prototype = Object.create(Lexer.prototype, {
    constructor: {value: PlainTextLexer},
    doColor: {value: function(hint) { return hint; }}
  });

  function inheritMode(ctor) {
    ctor.keymap = new Map();
    ctor.prototype = Object.create(Mode.prototype, {
      constructor: {value: ctor}
    });
  }

  /** @constructor @extends {Mode} @return {?} */
  function ConfigMode() {
    Mode.call(this, 'Config', ConfigLexer);
  }
  inheritMode(ConfigMode);

  /** @constructor @extends {Mode} @return {?} */
  function CppMode() {
    Mode.call(this, 'C++', CppLexer);
  }
  inheritMode(CppMode);

  /** @constructor @extends {Mode} @return {?} */
  function IdlMode() {
    Mode.call(this, 'IDL', IdlLexer);
  }
  inheritMode(IdlMode);

  /** @constructor @extends {Mode} @return {?} */
  function JavaMode() {
    Mode.call(this, 'Java', JavaLexer);
  }
  inheritMode(JavaMode);

  /** @constructor @extends {Mode} @return {?} */
  function JavaScriptMode() {
    Mode.call(this, 'JavaScript', JavaScriptLexer);
  }
  inheritMode(JavaScriptMode);

  /** @constructor @extends {Mode} @return {?} */
  function HaskellMode() {
    Mode.call(this, 'Haskell', PlainTextLexer);
  }
  inheritMode(HaskellMode);

  /** @constructor @extends {Mode} @return {?} */
  function LispMode() {
    Mode.call(this, 'Lisp', PlainTextLexer);
  }
  inheritMode(LispMode);

  /** @constructor @extends {Mode} @return {?} */
  function MasonMode() {
    Mode.call(this, 'Mason', PlainTextLexer);
  }
  inheritMode(MasonMode);

  /** @constructor @extends {Mode} @return {?} */
  function PerlMode() {
    Mode.call(this, 'Perl', PlainTextLexer);
  }
  inheritMode(PerlMode);

  /** @constructor @extends {Mode} @return {?} */
  function PlainTextMode() {
    Mode.call(this, 'Plain', PlainTextLexer);
  }
  inheritMode(PlainTextMode);

  /** @constructor @extends {Mode} @return {?} */
  function PythonMode() {
    Mode.call(this, 'Python', PythonLexer);
  }
  inheritMode(PythonMode);

  /** @constructor @extends {Mode} @return {?} */
  function XmlMode() {
    Mode.call(this, 'XML', PlainTextLexer);
  }
  inheritMode(XmlMode);

  Object.defineProperty(Mode, 'defaultMode', {
    /** @type {!Mode} */
    value: new PlainTextMode()
  });

  Object.defineProperty(Mode, 'extensionMap', {
    value: (function() {
      var map = /** @type{!ExtensionToModeMap} */(new Map());
      map.set('asdl', {mode: XmlMode, name: 'ASDL Document'});
      map.set('cc', {mode: CppMode, name: 'C++ Source'});
      map.set('cfg', {mode: ConfigMode, name: 'Config File'});
      map.set('cl', {mode: CppMode, name: 'CommonLisp Source'});
      map.set('cpp', {mode: CppMode, name: 'C++ Source'});
      map.set('css', {mode: CppMode, name: 'Cascading Style Sheet Document'});
      map.set('cxx', {mode: CppMode, name: 'C++ Source'});
      map.set('el', {mode: LispMode, name: 'EmacsLisp Source'});
      map.set('gyp', {mode: PythonMode, name: 'GYP Source'});
      map.set('gypi', {mode: PythonMode, name: 'GYP Include Source'});
      map.set('h', {mode: CppMode, name: 'C/C++ Header'});
      map.set('hs', {mode: HaskellMode, name: 'Haskell Source'});
      map.set('hsc', {mode: HaskellMode, name: 'Haskell Component Source'});
      map.set('hpp', {mode: CppMode, name: 'C++ Header'});
      map.set('htm', {mode: XmlMode, name: 'HTML Document'});
      map.set('html', {mode: XmlMode, name: 'HTML Document'});
      map.set('hxx', {mode: CppMode, name: 'C++ Header'});
      map.set('idl', {mode: IdlMode, name: 'Web IDL Source'});
      map.set('java', {mode: JavaMode, name: 'Java Source'});
      map.set('js', {mode: JavaScriptMode, name: 'JavaScript Source'});
      map.set('jsm', {mode: JavaScriptMode, name: 'JavaScript Module Source'});
      map.set('l', {mode: LispMode, name: 'Lisp Source'});
      map.set('lisp', {mode: LispMode, name: 'Lisp Source'});
      map.set('lsp', {mode: LispMode, name: 'Lisp Source'});
      map.set('m', {mode: MasonMode, name: 'Mason Source'});
      map.set('mi', {mode: MasonMode, name: 'Mason Include Source'});
      map.set('mk', {mode: ConfigMode, name: 'Config File'});
      map.set('mm', {mode: CppMode, name: 'Objective-C Source'});
      map.set('pl', {mode: PerlMode, name: 'Perl Source'});
      map.set('pm', {mode: PerlMode, name: 'Perl Module Source'});
      map.set('py', {mode: PythonMode, name: 'Python Source'});
      map.set('scm', {mode: LispMode, name: 'Scheme Source'});
      map.set('stanza', {mode: ConfigMode, name: 'Config File'});
      map.set('t', {mode: PerlMode, name: 'Perl Test Source'});
      map.set('wsdl', {mode: XmlMode, name: 'WSDL Document'});
      map.set('xhtml', {mode: XmlMode, name: 'XHTML Document'});
      map.set('xml', {mode: XmlMode, name: 'XML Document'});
      map.set('xsd', {mode: XmlMode, name: 'XSD Document'});
      map.set('xsl', {mode: XmlMode, name: 'XSLT Document'});
      return map;
    })()
  });

  Object.defineProperty(Mode, 'nameMap', {
    /** @type {!NameToModeMap} */
    value: (function() {
      var map = new Map();
      map.set('config', ConfigMode);
      map.set('c++', CppMode);
      map.set('haskell', HaskellMode);
      map.set('java', JavaMode);
      map.set('javascript', JavaScriptMode);
      map.set('lisp', LispMode);
      map.set('mason', MasonMode);
      map.set('perl', PerlMode);
      map.set('plain', PlainTextMode);
      map.set('python', PythonMode);
      map.set('xml', XmlMode);
      return /** @type {!NameToModeMap} */(map);
    })()
  });

  Object.defineProperty(Mode, 'fileNameMap', {
    /** @type {!NameToModeMap} */
    value: (function() {
        var map = new Map();
        map.set('Makefile', ConfigMode);
        map.set('autohandler', MasonMode);
        map.set('dhandler', MasonMode);
        map.set('makefile', ConfigMode);
        return /** @type {!NameToModeMap} */(map);
      })()
  });

  Object.defineProperty(Mode, 'chooseMode', {
    /**
     * @param {!Document} document
     * @return {!Mode}
     */
    value: function(document) {
      var mode_name = document.properties.get('mode') ||
                      document.properties.get('Mode');
      if (mode_name !== undefined) {
        var mode_ctor = Mode.nameMap.get(mode_name.toLowerCase());
        if (mode_ctor)
          return new mode_ctor();
      }
      if (document.fileName != '')
        return Mode.chooseModeByFileName(document.fileName);
      return Mode.chooseModeByFileName(document.name);
    }
  });

  Object.defineProperty(Mode, 'chooseModeByFileName', {
    /**
     * @param {string} fileName
     * @return {!Mode}
     */
    value: function(fileName) {
      var matches = /[.](.+)$/.exec(FilePath.basename(fileName));
      if (!matches) {
        var mode_ctor = Mode.fileNameMap.get(FilePath.basename(fileName));
        if (mode_ctor)
          return new mode_ctor();
        return new Mode.defaultMode.constructor();
      }
      var description = Mode.extensionMap.get(matches[1]);
      if (!description)
        return new Mode.defaultMode.constructor();
      return new description.mode();
    }
  });
})();
