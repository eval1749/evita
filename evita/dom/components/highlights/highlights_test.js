// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {

const StateRangeMap = highlights.StateRangeMap;
const Token = highlights.Token;

/** @const @type {!Map<stirng, string>} */
const kSyntaxMap = new Map();
kSyntaxMap.set('html_attribute_name', 'attribute');
kSyntaxMap.set('html_attribute_value', 'value');
kSyntaxMap.set('html_element_name', 'element');
kSyntaxMap.set('html_entity', '&');
kSyntaxMap.set('normal', '_');

function encodeSyntax(syntax) {
  return kSyntaxMap.get(syntax) || syntax;
}

function testPaint(painterCreator, stateMachine, text) {
  const document = new TextDocument();
  const painter = painterCreator.call(this, document);
  const tokenizer = new highlights.Tokenizer(document, painter, stateMachine);
  const range = new TextRange(document);
  range.text = text;
  tokenizer.doColor(document.length);
  const result = [];
  let tokenSyntax = '';
  let tokenLength = 0;
  for (let offset = 0; offset < document.length; ++offset) {
    const syntax = encodeSyntax(document.syntaxAt(offset));
    if (tokenSyntax === '') {
      tokenSyntax = syntax;
      tokenLength = 1;
      continue;
    }
    if (tokenSyntax !== syntax) {
      result.push(`${tokenSyntax[0]}${tokenLength}`);
      tokenSyntax = syntax;
      tokenLength = 1;
      continue;
    }
    ++tokenLength;
  }
  if (tokenSyntax !== '')
    result.push(`${tokenSyntax[0]}${tokenLength}`);
  return result.join(' ');
}

function testScan(machine, sample) {
  const stateRanges = [];
  function endStateRange(stateRange, offset) {
    stateRange.end = offset;
    stateRange.syntax = encodeSyntax(machine.syntaxOf(stateRange.state));
    stateRanges.push(stateRange);
  }

  function updateState(charCode) {
    const state = machine.updateState(charCode);
    if (state !== 0)
      return state;
    // When |state| is zero, the last state is an acceptable state and
    // no more consumes input. Thus, we need to compute new state with
    // current input.
    return machine.updateState(charCode);
  }

  machine.resetTo(0);
  machine.updateState(Unicode.LF);
  let stateRange = null;
  let offset = 0;
  for (const char of sample) {
    const lastState = machine.state;
    const charCode = char.charCodeAt(0);
    const state = updateState(charCode);
    if (state === lastState && stateRange) {
      ++offset;
      continue;
    }
    if (stateRange)
      endStateRange(stateRange, offset);
    if (machine.isAcceptable(state))
      machine.resetTo(0);
    stateRange = {start: offset, end: offset + 1, state: state};
    ++offset;
  }
  endStateRange(stateRange, offset);

  return tokenize(stateRanges)
      .map(token => `${token.code}${token.end - token.start}`)
      .join(' ');
}

function tokenize(ranges) {
  const tokens = [];
  for (const range of ranges) {
    if (tokens.length === 0) {
      tokens.push({
        code: range.syntax[0],
        end: range.end,
        start: range.start,
        syntax: range.syntax,
      });
      continue;
    }
    const token = tokens[tokens.length - 1];
    if (token.syntax === '') {
      token.syntax = range.syntax;
      token.end = range.end;
      continue;
    }
    if (token.syntax === range.syntax) {
      token.end = range.end;
      continue;
    }
    tokens.push({
      code: range.syntax[0],
      end: range.end,
      start: range.start,
      syntax: range.syntax,
    });
    continue;
  }
  return tokens;
}

testing.test('CppStateRangeStateMachine', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const scan = testScan.bind(this, machine);
  t.expect(scan('/* abc */ def')).toEqual('c9 w1 i3');
  t.expect(scan('"abc"')).toEqual('s5');
  t.expect(scan('\'abc\'')).toEqual('s5');
  t.expect(scan('#if')).toEqual('i3');
  t.expect(scan('  #  if'), 'Leading whitespace is tokenized into "whitespace"')
      .toEqual('w2 i5');
  t.expect(scan('/**/')).toEqual('c4');
  t.expect(scan('\x2F/*')).toEqual('c3');
  t.expect(
       scan('\x2F/ xyz\\\nbar'),
       'Backslash before newline continues line comment to next line')
      .toEqual('c11');
});

testing.test('CmdPainter', function(t) {
  const machine = new highlights.CmdTokenStateMachine();
  const paint = testPaint.bind(this, highlights.CmdPainter.create, machine);
  t.expect(paint('dir')).toEqual('k3');
  t.expect(paint('if')).toEqual('k2');
  t.expect(paint('rmdir foo')).toEqual('k5 _4');
  t.expect(paint('foo')).toEqual('i3');
  t.expect(paint('  regex')).toEqual('_2 i5');
  t.expect(paint('"foo"')).toEqual('_5');
  t.expect(paint('rem remark')).toEqual('k3 c7');
  t.expect(paint(': comment')).toEqual('c9');
  t.expect(paint(':label')).toEqual('l6');
  t.expect(paint('%1')).toEqual('_2');
  t.expect(paint('%var%')).toEqual('_5');
});

testing.test('ConfigPainter', function(t) {
  const machine = new highlights.ConfigTokenStateMachine();
  const paint = testPaint.bind(this, highlights.ConfigPainter.create, machine);
  t.expect(
       paint(
           '# foo\n' +
           'all: object\n' +
           '  echo foo\n'))
      .toEqual('c5 w1 i3 o1 w1 i6 w3 i4 w1 i3 w1');
});

testing.test('CppPainter', function(t) {
  const machine = new highlights.CppTokenStateMachine();
  const paint = testPaint.bind(this, highlights.CppPainter.create, machine);
  t.expect(paint('default:')).toEqual('k7 o1');
  t.expect(paint('foo:')).toEqual('l3 o1');
  t.expect(paint('foo::')).toEqual('i5');
  t.expect(paint('foo::bar')).toEqual('i8');
  t.expect(paint('foo::bar:')).toEqual('l8 o1');
  t.expect(paint('foo::bar::')).toEqual('i10');

  t.expect(paint('/* foo */')).toEqual('c9');
  t.expect(paint('// bar\nfoo')).toEqual('c6 w1 i3');
  t.expect(paint('#include <foo>')).toEqual('k8 w1 o1 i3 o1');
  t.expect(paint('auto a1 = \'s1\';')).toEqual('k4 w1 i2 w1 o1 w1 s4 o1');
  t.expect(paint('auto a2 = "s2";')).toEqual('k4 w1 i2 w1 o1 w1 s4 o1');
  t.expect(paint('std::unique_ptr<T>'), 'keyword').toEqual('k15 o1 i1 o1');
  t.expect(paint('std::vector_ptr<T>'), 'not keyword').toEqual('i15 o1 i1 o1');
  t.expect(paint('base::string16'), 'not keyword').toEqual('i14');
});

testing.test('CSharpPainter', function(t) {
  const machine = new highlights.CSharpTokenStateMachine();
  const paint = testPaint.bind(this, highlights.CSharpPainter.create, machine);
  t.expect(paint('default:')).toEqual('k8');
  t.expect(paint('if')).toEqual('k2');
  t.expect(paint('var')).toEqual('k3');
  t.expect(paint('foo')).toEqual('i3');
  t.expect(paint('/* foo */')).toEqual('c9');
  t.expect(paint('// bar\nfoo')).toEqual('c6 w1 i3');
  t.expect(paint('\'s1\';')).toEqual('s4 o1');
  t.expect(paint('"s2";')).toEqual('s4 o1');
  t.expect(paint('@"foo "" bar";')).toEqual('s13 o1');
  t.expect(paint('Math.Sin(1)')).toEqual('k8 o1 z1 o1');
  t.expect(paint('this.foo')).toEqual('k4 i4');
  t.expect(paint('this.GetType()')).toEqual('k4 o1 k7 o2');
  t.expect(paint('Console.Write()')).toEqual('k13 o2');
});

testing.test('CssPainter', function(t) {
  const machine = new highlights.CssTokenStateMachine();
  const paint = testPaint.bind(this, highlights.CssPainter.create, machine);
  t.expect(paint('color: #222')).toEqual('k6 w1 z4');
  t.expect(paint('foo: initial')).toEqual('l4 w1 k7');
  t.expect(paint('foo: "bar"')).toEqual('l4 w1 s5');
  t.expect(paint('foo: \'bar\'')).toEqual('l4 w1 s5');
});

testing.test('GnPainter', function(t) {
  const machine = new highlights.GnTokenStateMachine();
  const paint = testPaint.bind(this, highlights.GnPainter.create, machine);
  t.expect(
       paint(
           '# foo\n' +
           'source_set("bar") {\n' +
           '  baz = true\n' +
           '}\n'))
      .toEqual('c5 w1 k10 o1 s5 o1 w1 o1 w3 i3 w1 o1 w1 k4 w1 o1 w1');
});

testing.test('HtmlPainter', function(t) {
  const machine = new highlights.HtmlTokenStateMachine();
  const paint = testPaint.bind(this, highlights.HtmlPainter.create, machine);
  t.expect(paint('<!doctype html>foo')).toEqual('k15 _3');
  t.expect(
       paint(
           '<script>\n' +
           'var x = "foo";\n' +
           '</script>\n' +
           '<div>foo</div>'))
      .toEqual('k1 e6 k1 a25 _1 k1 e3 k1 _3 k2 e3 k1');
});

testing.test('IdlPainter', function(t) {
  const machine = new highlights.IdlTokenStateMachine();
  const paint = testPaint.bind(this, highlights.IdlPainter.create, machine);
  t.expect(
       paint(
           '/* foo */\n' +
           '// bar\n' +
           'attribute a1 = "s1";\n' +
           'interface a2 {};\n'))
      .toEqual('c9 w1 c6 w1 k9 w1 i2 w1 o1 w1 s4 o1 w1 k9 w1 i2 w1 o3 w1');
});

testing.test('JavaScriptPainter', function(t) {
  const machine = new highlights.JavaScriptTokenStateMachine();
  const paint =
      testPaint.bind(this, highlights.JavaScriptPainter.create, machine);
  t.expect(paint('default:')).toEqual('k8');
  t.expect(paint('if')).toEqual('k2');
  t.expect(paint('var')).toEqual('k3');
  t.expect(paint('foo')).toEqual('i3');
  t.expect(paint('/* foo */')).toEqual('c9');
  t.expect(paint('// bar\nfoo')).toEqual('c6 w1 i3');
  t.expect(paint('\'s1\';')).toEqual('s4 o1');
  t.expect(paint('"s2";')).toEqual('s4 o1');
  t.expect(paint('`s2`;')).toEqual('s4 o1');
  t.expect(paint('Math.sin(1)')).toEqual('k8 o1 z1 o1');
  t.expect(paint('this.foo')).toEqual('k4 i4');
  t.expect(paint('this.length')).toEqual('k4 o1 k6');
  t.expect(paint('Foo.length')).toEqual('i3 o1 k6');
  t.expect(paint('Foo.prototype')).toEqual('i3 o1 k9');
  t.expect(paint('Foo.toString()')).toEqual('i3 o1 k8 o2');
});

testing.test('JavaPainter', function(t) {
  const machine = new highlights.JavaTokenStateMachine();
  const paint = testPaint.bind(this, highlights.JavaPainter.create, machine);
  t.expect(paint('default:')).toEqual('k8');
  t.expect(paint('if')).toEqual('k2');
  t.expect(paint('foo')).toEqual('i3');
  t.expect(paint('/* foo */')).toEqual('c9');
  t.expect(paint('// bar\nfoo')).toEqual('c6 w1 i3');
  t.expect(paint('\'s1\';')).toEqual('s4 o1');
  t.expect(paint('"s2";')).toEqual('s4 o1');
  t.expect(paint('Math.sin(1)')).toEqual('k8 o1 z1 o1');
  t.expect(paint('this.foo')).toEqual('k4 i4');
  t.expect(paint('this.getClass()')).toEqual('k4 o1 k8 o2');
  t.expect(paint('@Override\n')).toEqual('k9 w1');
  t.expect(paint('@SuppressWarnings\n')).toEqual('k17 w1');
});

testing.test('PlainPainter', function(t) {
  const machine = new highlights.PlainTokenStateMachine();
  const paint = testPaint.bind(this, highlights.PlainPainter.create, machine);
  t.expect(paint('foo bar baz')).toEqual('z3 w1 z3 w1 z3');
  t.expect(paint(' http://foo.bar ')).toEqual('w1 k14 w1');
});

testing.test('PythonPainter', function(t) {
  const machine = new highlights.PythonTokenStateMachine();
  const paint = testPaint.bind(this, highlights.PythonPainter.create, machine);
  t.expect(paint('# comment')).toEqual('c9');
  t.expect(paint('"string"')).toEqual('s8');
  t.expect(paint('"""string"""')).toEqual('s12');
  t.expect(paint('\'string\'')).toEqual('s8');
  t.expect(paint('def foo(x):')).toEqual('k3 w1 i3 o1 i1 o2');
  t.expect(paint('self.value = 2')).toEqual('k4 i6 w1 o1 w1 z1');
  t.expect(paint('if a1 == 12')).toEqual('k2 w1 i2 w1 o2 w1 z2');
  t.expect(
       paint(
           '# bar\n' +
           '# foo\n' +
           'if a1 == \'s1\':\n' +
           '    a2 = "s2"\n'))
      .toEqual('c5 w1 c5 w1 k2 w1 i2 w1 o2 w1 s4 o1 w5 i2 w1 o1 w1 s4 w1');
});


testing.test('StateRangeMap', function(t) {
  function makeSample(headCount, tailCount, delta) {
    const doc = new TextDocument();
    const map = new StateRangeMap(doc);
    const range = new TextRange(doc);
    //            012345678
    range.text = '<a>bc</a>';
    const stag = new Token(doc, 0, 3, 'stag');
    const cont = new Token(doc, 3, 5, 'cont');
    const etag = new Token(doc, 5, 9, 'etag');
    map.add(0, 1, 1, stag);
    map.add(1, 2, 2, stag);
    map.add(2, 3, 3, stag);
    map.add(3, 5, 4, cont);
    map.add(5, 7, 5, etag);
    map.add(7, 8, 6, etag);
    map.add(8, 9, 7, etag);
    map.didChangeTextDocument(headCount, tailCount, delta);
    return dumpMap(map);
  }

  function dumpMap(map) {
    const result = [];
    for (const range of map.ranges())
      result.push(`(${range.start} ${range.end} ${range.syntax})`);
    return result.join(' ');
  }

  t.expect(makeSample(0, 8, 1), 'insert at start')
      .toEqual('(4 6 cont) (6 8 etag) (8 9 etag) (9 10 etag)');
  t.expect(makeSample(2, 6, 1), 'insert before ">"@2')
      .toEqual(
          '(0 1 stag) (1 2 stag) (4 6 cont) (6 8 etag) (8 9 etag) (9 10 etag)');
  t.expect(makeSample(8, 1, 1), 'insert at end')
      .toEqual(
          '(0 1 stag) (1 2 stag) (2 3 stag) (3 5 cont) (5 7 etag) (7 8 etag)' +
          ' (9 10 etag)');
  t.expect(makeSample(0, 7, -1), 'remove "<"@0')
      .toEqual('(4 6 etag) (6 7 etag) (7 8 etag)');
  t.expect(makeSample(2, 6, -1), 'remove ">"@2')
      .toEqual('(0 1 stag) (1 2 stag) (4 6 etag) (6 7 etag) (7 8 etag)');
});

testing.test('XmlPainter', function(t) {
  const machine = new highlights.XmlTokenStateMachine();
  const paint = testPaint.bind(this, highlights.XmlPainter.create, machine);
  t.expect(paint('<foo id="12">')).toEqual('k1 e3 _1 a2 k2 v2 k2');
  t.expect(paint('<foo id=\'12\'>')).toEqual('k1 e3 _1 a2 k2 v2 k2');
  t.expect(paint('<foo id="1"/>')).toEqual('k1 e3 _1 a2 k2 v1 k3');
  t.expect(paint('<foo id=foo/>')).toEqual('k1 e3 _1 a2 k1 v3 k2');

  t.expect(paint('<!doctype html>foo')).toEqual('k15 _3');

  // XML keywords
  t.expect(paint('<p:foo xml:p="ns-uri">')).toEqual('k1 e5 _1 a5 k2 v6 k2');
  t.expect(paint('<foo xmlns:>')).toEqual('k1 e3 _1 k7');
  t.expect(paint('<xi:include href="foo">')).toEqual('k11 _1 a4 k2 v3 k2');

  t.expect(paint('<br />')).toEqual('k1 e2 _1 k2');
  t.expect(
       paint(
           '<!-- comment -->\n' +
           '<element attr1="123" attr2="456">\n' +
           'foo&amp;bar\n' +
           '</element>'))
      .toEqual('c16 _1 k1 e7 _1 a5 k2 v3 k1 _1 a5 k2 v3 k2 _4 &5 _4 k2 e7 k1');
  t.expect(paint('& && &amp; &#123; &#xBEEF; foo'))
      .toEqual('&1 _1 &2 _1 &5 _1 &6 _1 &8 _4');
  t.expect(paint('<abc def="123"ghi>'), 'No space between attributes')
      .toEqual('k1 e3 _1 a3 k2 v3 k1 _4');
  t.expect(paint('<abc def="123'), 'No ending quote')
      .toEqual('k1 e3 _1 a3 k2 v3');
});

});
