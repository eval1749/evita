# highlights - Syntax highlighting component

Syntax highlighting engine uses regular expression patterns to extract tokens
from text document. Regular expression patterns are compiled into DFA,
Deterministic Finite Automata, for fast pattern matching.

## Things for syntax highlighting engine
To implement syntax syntax highlighting for a language, you need to provide
followings:
 1. Syntax highlighting rule file, e.g. `foo_tokens.xml`
 2. Highlighter extends `highlights.base.Highlighter`, e.g. `FooHighlighter`
 3. (Optional) Painter extends `highlights.base.Painter`, e.g. `FooPainter`


## Custom Painting
To implement custom painting, you need to implement customer painter class
derived from |highlights.Paint| class.

|highlights.Paint| class provides following method for implement customer
painter:
 * `paint(token)`; the entry point of custom painting, your derived class
 should implements this method.
 * `textOf(token): string`; extract text for `token`.
 * `paintToken(token)`; paints a token with `token.syntax`.
 * `previousToken(token): token`; returns previous token of `token` or null
 if there is no such token, e.g. start of document, start of fragment, etc.

## Sample implementation
Following code implements syntax highlighting engine for language "Foo":

*foo_tokens.xml*
```xml
<tokens name="foo">
  <!-- C++ like block comment -->
  <token name="block_comment" type="comment"
         pattern="/[*][^*]*[*]+([^/*][^*]*[*]+)*/" />
  <!-- C++ like line comment -->
  <token name="line_comment" type="comment"
         pattern="//([^\n]|\\.)*" />
  <!-- Double-quoted string -->
  <token name="string2" type="string_literal"
         pattern='"([^\\"]|\\.)*"' />
  <!-- Identifiers start with letter or underscore following alphabet or digit.
    -->
  <token name="name" type="identifier"
         pattern="[A-Za-z_]\w*" />
</tokens>
```

*foo_highlighter.xml*
```javascript
// FooTokenStateMachine is generated from "foo_tokens.xml"
const FooTokenStateMachine = highlights.FooTokenStateMachine

// Imports |highlights.base| symbols
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Tokenizer = highlights.base.Tokenizer;

const keywords = Set(['foo', 'bar', 'baz']);

// Optional painter
class FooPainter extends Painter {
  constructor(tokenMap) {
    super(tokenMap);
  }

  // Overrides |Painter.paint()| for custom painting of token.
  paint(token) {
    if (token.syntax === 'identifier') {
      if (keywords.has(this.textOf(token))
        return this.setSyntax(token, 'keyword');
    }
    this.paintToken(token);
  }

  static create(tokenMap) {
    return new FooPainter(tokenMap);
  }
}

class FooHighlighter extends highlights.base.Highlighter {
  constructor(document) {
    super(document, FooPainter.create, new FooTokenStateMachine());
  }
}
```
