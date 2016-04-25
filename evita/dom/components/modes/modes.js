// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('modes');

goog.scope(function() {

const Mode = modes.Mode;

Mode.registerMode('cmd', 'CommandScript');
Mode.registerMode('config', 'Config');
Mode.registerMode('c++', 'C++');
Mode.registerMode('c#', 'C#');
Mode.registerMode('css', 'CSS');
Mode.registerMode('gn', 'Gn');
Mode.registerMode('html', 'HTML');
Mode.registerMode('idl', 'IDL');
Mode.registerMode('java', 'Java');
Mode.registerMode('javascript', 'JavaScript');
Mode.registerMode('plain', 'Plain');
Mode.registerMode('python', 'Python');
Mode.registerMode('xml', 'XML');

Mode.registerModeAlias('haskell', 'Haskell', 'plain');
Mode.registerModeAlias('lisp', 'Lisp', 'plain');
Mode.registerModeAlias('mason', 'Mason', 'html');
Mode.registerModeAlias('perl', 'Perl', 'python');

Mode.registerExtension('asdl', 'xml', 'ASDL Document');
Mode.registerExtension('bat', 'cmd', 'Windows Command Script');
Mode.registerExtension('cc', 'c++', 'C++ Source');
Mode.registerExtension('cfg', 'config', 'Config File');
Mode.registerExtension('cl', 'c++', 'CommonLisp Source');
Mode.registerExtension('cmd', 'cmd', 'Windows Command Script');
Mode.registerExtension('cpp', 'c++', 'C++ Source');
Mode.registerExtension('cs', 'c#', 'C# Source');
Mode.registerExtension('css', 'css', 'Cascading Style Sheet Document');
Mode.registerExtension('cxx', 'c++', 'C++ Source');
Mode.registerExtension('el', 'lisp', 'EmacsLisp Source');
Mode.registerExtension('gn', 'gn', 'GN Source');
Mode.registerExtension('gni', 'gn', 'GN Include Source');
Mode.registerExtension('gyp', 'python', 'GYP Source');
Mode.registerExtension('gypi', 'python', 'GYP Include Source');
Mode.registerExtension('h', 'c++', 'C/C++ Header');
Mode.registerExtension('hs', 'haskell', 'Haskell Source');
Mode.registerExtension('hsc', 'haskell', 'Haskell Component Source');
Mode.registerExtension('hpp', 'c++', 'C++ Header');
Mode.registerExtension('htm', 'html', 'HTML Document');
Mode.registerExtension('html', 'html', 'HTML Document');
Mode.registerExtension('hxx', 'c++', 'C++ Header');
Mode.registerExtension('idl', 'idl', 'Web IDL Source');
Mode.registerExtension('java', 'java', 'Java Source');
Mode.registerExtension('js', 'javascript', 'JavaScript Source');
Mode.registerExtension('jsm', 'javascript', 'JavaScript Module Source');
Mode.registerExtension('json', 'javascript', 'JSON File');
Mode.registerExtension('l', 'lisp', 'Lisp Source');
Mode.registerExtension('lisp', 'lisp', 'Lisp Source');
Mode.registerExtension('lsp', 'lisp', 'Lisp Source');
Mode.registerExtension('m', 'mason', 'Mason Source');
Mode.registerExtension('mi', 'mason', 'Mason Include Source');
Mode.registerExtension('mk', 'config', 'Config File');
Mode.registerExtension('mm', 'c++', 'Objective-C Source');
Mode.registerExtension('pl', 'perl', 'Perl Source');
Mode.registerExtension('pm', 'perl', 'Perl Module Source');
Mode.registerExtension('py', 'python', 'Python Source');
Mode.registerExtension('scm', 'lisp', 'Scheme Source');
Mode.registerExtension('stanza', 'config', 'Config File');
Mode.registerExtension('t', 'perl', 'Perl Test Source');
Mode.registerExtension('wsdl', 'xml', 'WSDL Document');
Mode.registerExtension('xhtml', 'html', 'XHTML Document');
Mode.registerExtension('xml', 'xml', 'XML Document');
Mode.registerExtension('xsd', 'xml', 'XSD Document');
Mode.registerExtension('xsl', 'xml', 'XSLT Document');

Mode.registerFileName('DEPS', 'python');
Mode.registerFileName('Makefile', 'config');
Mode.registerFileName('autohandler', 'mason');
Mode.registerFileName('dhandler', 'mason');
Mode.registerFileName('makefile', 'config');

class Observer extends SimpleTextDocumentSetObserver {
  constructor() { super(); }

  /** @param {!TextDocument} document */
  didAddTextDocument(document) {
    document.addEventListener(
        Event.Names.LOAD, this.didLoadTextDocument.bind(document));
  }

  /**
   * @this {!TextDocument}
   *
   * Updates document mode by mode property in contents or file name.
   */
  didLoadTextDocument() {
    /** @type {!TextDocument} */
    const document = this;
    document.parseFileProperties();
    /** @type {!Mode} */
    const newMode = Mode.chooseMode(document);
    /** @type {string} */
    const currentModeName = document.mode ? document.mode.name : '';
    if (newMode.name === currentModeName)
      return;
    Editor.messageBox(
        null, `Change mode to ${newMode.name}`, MessageBox.ICONINFORMATION);
    document.mode = /** @type {Mode} */ (newMode);
  }

  /** @param {!TextDocument} document */
  didRemoveTextDocument(document) { document.mode = null; }
}

TextDocument.addObserver(new Observer());

});
