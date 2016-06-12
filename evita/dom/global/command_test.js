// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('commands');
goog.require('testing');

goog.scope(() => {
const Command = commands.Command;

testing.test('Command.register', (t) => {
  const command = new Command('foo', () => {}, 'Foo command');
  Command.register(command);
  t.expect(command.name).toEqual('foo');
  t.expect(command.description).toEqual('Foo command');
  t.expect(Command.query('foo')).toEqual(command);
});
});
