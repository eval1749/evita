// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  class PlainTextLexer extends global.Lexer {
    /**
     * @param {!TextDocument} document
     */
    constructor(document) { super(document, {}); }
    detach() {}
    doColor(hint) {}
  }

  function inheritMode(ctor) {
    ctor.keymap = new Map();
  }

  class ConfigMode extends global.Mode {
    constructor() {
      super('Config', ConfigLexer);
    }
  }
  inheritMode(ConfigMode);

  class CppMode  extends global.Mode {
    constructor() {
      super('C++', CppLexer);
    }
  }
  inheritMode(CppMode);

  class CsharpMode  extends global.Mode {
    constructor() {
      super('C#', CsharpLexer);
    }
  }
  inheritMode(CsharpMode);

  class HtmlMode  extends global.Mode {
    constructor() {
      super('HTML', HtmlLexer);
    }
  }
  inheritMode(HtmlMode);

  class IdlMode  extends global.Mode {
    constructor() {
      super('IDL', IdlLexer);
    }
  }
  inheritMode(IdlMode);

  class JavaMode  extends global.Mode {
    constructor() {
      super('Java', JavaLexer);
    }
  }
  inheritMode(JavaMode);

  class JavaScriptMode extends global.Mode {
    constructor() {
      super('JavaScript', JavaScriptLexer);
    }
  }
  inheritMode(JavaScriptMode);

  class HaskellMode extends global.Mode {
    constructor() {
      super('Haskell', PlainTextLexer);
    }
  }
  inheritMode(HaskellMode);

  class LispMode extends global.Mode {
    constructor() {
      super('Lisp', PlainTextLexer);
    }
  }
  inheritMode(LispMode);

  class MasonMode extends global.Mode {
    constructor() {
      super('Mason', PlainTextLexer);
    }
  }
  inheritMode(MasonMode);

  class PerlMode extends global.Mode {
    constructor() {
      super('Perl', PlainTextLexer);
    }
  }
  inheritMode(PerlMode);

  class PlainTextMode extends global.Mode {
    constructor() {
      super('Plain', PlainTextLexer);
    }
  }
  inheritMode(PlainTextMode);

  class PythonMode extends global.Mode {
    constructor() {
      super('Python', PythonLexer);
    }
  }
  inheritMode(PythonMode);

  class XmlMode extends global.Mode {
    constructor() {
      super('XML', XmlLexer);
    }
  }
  inheritMode(XmlMode);

  Object.defineProperty(Mode, 'defaultMode', {
    /** @type {!Mode} */
    value: new PlainTextMode()
  });

  Object.defineProperty(Mode, 'extensionMap', {
    value: (function() {
      var map = /** @type{!ExtensionToModeMap} */(new Map());
      map.set('asdl', {mode: XmlMode, name: 'ASDL TextDocument'});
      map.set('cc', {mode: CppMode, name: 'C++ Source'});
      map.set('cfg', {mode: ConfigMode, name: 'Config File'});
      map.set('cl', {mode: CppMode, name: 'CommonLisp Source'});
      map.set('cpp', {mode: CppMode, name: 'C++ Source'});
      map.set('cs', {mode: CsharpMode, name: 'C# Source'});
      map.set('css', {mode: CppMode, name: 'Cascading Style Sheet TextDocument'});
      map.set('cxx', {mode: CppMode, name: 'C++ Source'});
      map.set('el', {mode: LispMode, name: 'EmacsLisp Source'});
      map.set('gyp', {mode: PythonMode, name: 'GYP Source'});
      map.set('gypi', {mode: PythonMode, name: 'GYP Include Source'});
      map.set('h', {mode: CppMode, name: 'C/C++ Header'});
      map.set('hs', {mode: HaskellMode, name: 'Haskell Source'});
      map.set('hsc', {mode: HaskellMode, name: 'Haskell Component Source'});
      map.set('hpp', {mode: CppMode, name: 'C++ Header'});
      map.set('htm', {mode: HtmlMode, name: 'HTML TextDocument'});
      map.set('html', {mode: HtmlMode, name: 'HTML TextDocument'});
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
      map.set('wsdl', {mode: XmlMode, name: 'WSDL TextDocument'});
      map.set('xhtml', {mode: HtmlMode, name: 'XHTML TextDocument'});
      map.set('xml', {mode: XmlMode, name: 'XML TextDocument'});
      map.set('xsd', {mode: XmlMode, name: 'XSD TextDocument'});
      map.set('xsl', {mode: XmlMode, name: 'XSLT TextDocument'});
      return map;
    })()
  });

  Object.defineProperty(Mode, 'nameMap', {
    /** @type {!NameToModeMap} */
    value: (function() {
      var map = new Map();
      map.set('config', ConfigMode);
      map.set('c#', CsharpMode);
      map.set('c++', CppMode);
      map.set('haskell', HaskellMode);
      map.set('html', HtmlMode);
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
        map.set('DEPS', PythonMode);
        map.set('Makefile', ConfigMode);
        map.set('autohandler', MasonMode);
        map.set('dhandler', MasonMode);
        map.set('makefile', ConfigMode);
        return /** @type {!NameToModeMap} */(map);
      })()
  });

  Object.defineProperty(Mode, 'chooseMode', {
    /**
     * @param {!TextDocument} document
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
      if (document.fileName !== '')
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

  /** @param {!TextDocument} document */
  function didAddTextDocument(document) {
    document.addEventListener(Event.Names.BEFORELOAD,
                              willLoadTextDocument.bind(document));
    document.addEventListener(Event.Names.LOAD,
                              didLoadTextDocument.bind(document));
  }

  /**
   * @this {!TextDocument}
   *
   * Updates document mode by mode property in contents or file name.
   */
  function didLoadTextDocument() {
    /** @type {!TextDocument} */
    const document = this;
    document.parseFileProperties();
    /** @type {!Mode} */
    const newMode = Mode.chooseMode(document);
    /** @type {string} */
    const currentModeName = document.mode ? document.mode.name : '';
    if (newMode.name === currentModeName)
      return;
    Editor.messageBox(null, `Change mode to ${newMode.name}`,
                      MessageBox.ICONINFORMATION);
    document.mode = newMode;
  }

  /** @param {!TextDocument} document */
  function didRemoveTextDocument(document) {
    document.mode = null;
  }

  /** @this {!TextDocument} */
  function willLoadTextDocument() {
    const document = this;
  }

  TextDocument.addObserver(function(action, document) {
    switch (action) {
      case 'add':
        didAddTextDocument(document);
        break;
      case 'remove':
        didRemoveTextDocument(document);
        break;
    }
  });
})();
