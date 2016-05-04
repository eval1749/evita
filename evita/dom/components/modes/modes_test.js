// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('testing');

goog.scope(function() {

function modeFromProperty(mode_name) {
  const doc = new TextDocument();
  doc.properties.set('mode', mode_name);
  return Mode.chooseMode(doc).name;
}

function modeFromFileName(fileName) {
  return Mode.chooseModeByFileName(fileName).name;
}

testing.test('FromProperty', function(t) {
  t.expect(modeFromProperty('C++')).toEqual('C++');
  t.expect(modeFromProperty('Lisp')).toEqual('Lisp');
});

testing.test('FromFileName', function(t) {
  t.expect(modeFromFileName('foo.c')).toEqual('C++');
  t.expect(modeFromFileName('foo.cc')).toEqual('C++');
  t.expect(modeFromFileName('foo.cpp')).toEqual('C++');
  t.expect(modeFromFileName('foo.cxx')).toEqual('C++');
  t.expect(modeFromFileName('foo.h')).toEqual('C++');
  t.expect(modeFromFileName('foo.hpp')).toEqual('C++');
  t.expect(modeFromFileName('foo.hs')).toEqual('Haskell');
  t.expect(modeFromFileName('foo.htm')).toEqual('HTML');
  t.expect(modeFromFileName('foo.html')).toEqual('HTML');
  t.expect(modeFromFileName('foo.java')).toEqual('Java');
  t.expect(modeFromFileName('foo.js')).toEqual('JavaScript');
  t.expect(modeFromFileName('foo.l')).toEqual('Lisp');
  t.expect(modeFromFileName('foo.m')).toEqual('Mason');
  t.expect(modeFromFileName('foo.mk')).toEqual('Config');
  t.expect(modeFromFileName('foo.pl')).toEqual('Perl');
  t.expect(modeFromFileName('foo.py')).toEqual('Python');
  t.expect(modeFromFileName('foo.txt')).toEqual('Plain');
  t.expect(modeFromFileName('foo.xhtml')).toEqual('HTML');
  t.expect(modeFromFileName('foo.xml')).toEqual('XML');
});

});
