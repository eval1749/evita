// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

});
