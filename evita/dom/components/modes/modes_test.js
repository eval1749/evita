// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

function modeFromProperty(mode_name) {
  const doc = new TextDocument();
  doc.properties.set('mode', mode_name);
  return Mode.chooseMode(doc);
}

function modeFromFileName(fileName) {
  return Mode.chooseModeByFileName(fileName);
}

testing.test('FromProperty', function(t) {
  t.expect(modeFromProperty('C++')).toEqual('c++');
  t.expect(modeFromProperty('Lisp')).toEqual('plain');
});

testing.test('FromFileName', function(t) {
  t.expect(modeFromFileName('foo.c')).toEqual('c++');
  t.expect(modeFromFileName('foo.cc')).toEqual('c++');
  t.expect(modeFromFileName('foo.cpp')).toEqual('c++');
  t.expect(modeFromFileName('foo.cxx')).toEqual('c++');
  t.expect(modeFromFileName('foo.h')).toEqual('c++');
  t.expect(modeFromFileName('foo.hpp')).toEqual('c++');
  t.expect(modeFromFileName('foo.hs')).toEqual('haskell');
  t.expect(modeFromFileName('foo.htm')).toEqual('html');
  t.expect(modeFromFileName('foo.html')).toEqual('html');
  t.expect(modeFromFileName('foo.java')).toEqual('java');
  t.expect(modeFromFileName('foo.js')).toEqual('javascript');
  t.expect(modeFromFileName('foo.l')).toEqual('lisp');
  t.expect(modeFromFileName('foo.m')).toEqual('mason');
  t.expect(modeFromFileName('foo.mk')).toEqual('config');
  t.expect(modeFromFileName('foo.pl')).toEqual('perl');
  t.expect(modeFromFileName('foo.py')).toEqual('python');
  t.expect(modeFromFileName('foo.txt')).toEqual('plain');
  t.expect(modeFromFileName('foo.xhtml')).toEqual('html');
  t.expect(modeFromFileName('foo.xml')).toEqual('xml');
});

});
