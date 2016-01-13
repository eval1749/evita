// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

[
    // A
    'accessor', 'affine', 'allocator', 'allocatable', 'Allocatable',
    'Animatable', 'animatable', 'Animatables', 'animatables',
    'Arity', 'arity', 'ast',
    // B
    'bgcolor', 'boolean',
    // C
    'cacheable', 'Castable', 'callee', 'ceil', 'checkbox',
    'clazz','Collectable', 'contenteditable', 'css', 'ctor',
    // D
    'Decrement', 'decrement', 'destructor', 'doesn', 'dom', 'domapi',
    'Dominatee', 'dominatee', 'Dominator', 'dominator', 'dtor',
    // E
    'elang', 'emplace', 'endl', 'enum', 'evita', 'evaluator', 'Expander',
    'expander',
    // F
    'focusable', 'Formatter', 'formatter',
    // G
    'Getter', 'Getters', 'getter', 'getters', 'gfx',
    // H
    'hotlight', 'hwnd',
    // I
    'ime', 'initializer', 'Iterable', 'iterable',
    // J
    'javascript',
    // L
    'Layouter', 'layouter', 'lifecycle', 'Liveness', 'liveness',
    // N
    'nullable', 'nullptr',
    // O
    'Opcode', 'opcode', 'ostream', 'observee',
    // P
    'Posn', 'Prepend', 'pointee',
    // R
    'readonly', 'Rect', 'rect', 'Renderer', 'renderer',
    'Resize', 'resize', 'resizing', 'Runnable', 'runnable', 'runtime',
    // S
    'Scriptable', 'scriptable', 'scrollable', 'setter', 'Signedness',
    'signedness', 'splitter', 'stringify', 'substr', 'substring', 'subtree',
    'subtype', 'supertype',
    // T
    'Tooltip', 'tooltip', 'toplevel', 'Tuple', 'tuple', 'typedef',
    // U
    'unordered',
    // V
    'Validator', 'validator', 'Variadic', 'variadic', 'Visitable', 'visitable',
    'viewport',
    // W
    'waitable', 'whitespace',
    'yosi', 'yosin',
].forEach(function(word) {
  SpellChecker.keywords.add(word);
});

[
  // C
  'intptr_t', 'uintptr_t',

  // C++
  'std::all_of', 'std::any_of', 'std::array',
  'std::begin', 'std::bitset', 'std::boolalpha',
  'std::count', 'std::count_if',
  'std::deque',
  'std::end', 'std::endl',
  'std::find', 'std::find_if', 'std::for_each',
  'std::tie',
  'std::hash', 'std::hex',
  'std::ignore',
  'std::lower_bound',
  'std::make_pair', 'std::make_tuple', 'std::make_unique', 'std::mismatch',
  'std::next', 'std::none_of', 'std::noboolalpha',
  'std::ostream', 'std::ostringstream',
  'std::pair', 'std::prev',
  'std::queue',
  'std::stack', 'std::sort', 'std::stringstream',
  'std::tie', 'std::tuple',
  'std::upper_bound',

  // <type_traits>
  'std::add_constant',
  'std::add_cv',
  'std::add_extent',
  'std::add_lvalue_reference',
  'std::add_pointer',
  'std::add_rvalue_reference',
  'std::add_volatile',
  'std::aligned_storage',
  'std::aligned_union',
  'std::alignment_of',
  'std::common_type',
  'std::conditional',
  'std::decay',
  'std::enable_if',
  'std::extent',
  'std::false_type',
  'std::integral_constant',
  'std::is_base_of',
  'std::is_convertible',
  'std::is_same',
  'std::make_signed',
  'std::make_unsigned',
  'std::rank',
  'std::remove_all_extents',
  'std::remove_constant',
  'std::remove_cv',
  'std::remove_extent',
  'std::remove_pointer',
  'std::remove_reference',
  'std::remove_volatile',
  'std::result_of',
  'std::true_type',
  'std::underlying_type',

  // Chromium
  'DCHECK', 'DCHECK_EQ', 'DCHECK_NE', 'DCHECK_LE', 'DCHECK_LT', 'DCHECK_GE',
  'DCHECK_GT', 'NOTREACHED',
  'arraysize',
  'base::Bind',
  'base::Closure',
  'base::checked_cast',
  'base::char16',
  'base::string16', 'base::StringPiece', 'base::StringPiece16',
  'base::StringPrintf',
  'base::Time', 'base::TimeDelta', 'base::TimeTicks',
  'base::Unretained',
  'scoped_refptr',

  // v8
  'v8::ArrayBuffer',
  'v8::Context',
  'v8::EscapableHandleScope', 'v8::Eternal',
  'v8::FunctionCallback', 'v8::FunctionTemplate',
  'v8::Local', 'v8::HandleScope',
  'v8::Isolate',
  'v8::Local',
  'v8::Object', 'v8::ObjectTemplate',
  'v8::Persistent',
  'v8::Promise',
  'v8::Script',
  'v8::TryCatch',
  'v8::Undefined', 'v8::UniquePersistent',
  'v8::Value',

  // Vogue
  'domapi::EventType',
  'gfx::Canvas::AxisAlignedClipScope',
  'gfx::Brush',
  'gfx::Canvas', 'gfx::ColorF',
  'gfx::Canvas::DrawingScope',
  'gfx::Point', 'gfx::PointF',
  'gfx::Rect', 'gfx::RectF',
  'gfx::Size', 'gfx::SizeF',
].forEach((keyword) => {
  CppLexer.keywords.set(keyword, 'keyword');
});

function toCamelCase(document) {
  var underscore = new Editor.RegExp('\\b([a-z]+)(_[a-z])');
  var range = new Range(document);
  var offset = 0;
  while (offset < document.length) {
    var matches = document.match_(underscore, offset, document.length);
    if (!matches)
      break;
    range.collapseTo(matches[1].start);
    range.end = matches[1].end;
    if (range.text != 'opt') {
      range.collapseTo(matches[2].start);
      range.end = matches[2].end;
      range.text = range.text[1].toUpperCase();
    }
    offset = matches[0].end;
  }
}

Mode.extensionMap.set('e', Mode.extensionMap.get('cs'));
Mode.extensionMap.set('gn', Mode.extensionMap.get('py'));
Mode.extensionMap.set('gni', Mode.extensionMap.get('py'));

// Report spell checker progress
Editor.bindKey(TextWindow, 'Ctrl+Shift+M', function() {
  const document = this.document;
  const spellChecker = document.properties.get('SpellChecker');

  const coldScanner = spellChecker.coldScanner_;
  const coldOffset = coldScanner.offset;
  const coldEnd = coldScanner.end;

  const hotScanner = spellChecker.hotScanner_;
  const hotOffset = hotScanner.offset;
  const hotEnd= document.length;

  const message = `#Task=${taskScheduler.tasks_.size} Spell Checker(${document.name}) cold=${coldOffset}/${coldEnd} hot=${hotOffset}/${hotEnd}`;
  Editor.messageBox(this, message, 0);
  console.log(`/* ${message} */`);
});

// Trace logging Ctrl+Shift+T
(function() {
  const traceLogger = new TraceLogger();
  let running = false;

  function start(window) {
    Editor.messageBox(window, 'Start trace logger', 0);
    running = true;
    traceLogger.start();
  }

  function stop(window) {
    traceLogger.stop()
        .then(() => traceLogger.writeTo('d:/tmp/foo.txt'))
        .then((written) => {
          running = false;
          Editor.messageBox(window, `Wrote ${written} byte trace log`, 0);
        });
  }

  Editor.bindKey(TextWindow, 'Ctrl+Shift+T', function() {
    if (running) {
     stop(this);
     return;
    }
    start(this);
  });
})();

// Templates
(function() {
  const copyrightLines = [
      'Copyright (c) 2016 Project Vogue. All rights reserved.',
      'Use of this source code is governed by a BSD-style license that can be',
      'found in the LICENSE file.'
  ];

  const copyrightTextCxx = '// ' + copyrightLines.join('\n// ') + '\n\n';
  const copyrightTextSharp = '# ' + copyrightLines.join('\n# ') + '\n\n';

  const pythonTemplate = [
    copyrightTextSharp,
    'import os',
    'import sys',
    '',
    'def main():',
    '',
    '',
    'if __name__ == \'__main__\':',
    'sys.exit(main())',
    '',
  ].join('\n');

  function sourcePath(document) {
    const fileName = document.fileName || FilePath.fullPath(document.name);
    const path = FilePath.split(fileName).components;
    const srcIndex = path.findIndex((element, index) => {
      return index > 0 && element === 'src'
    });
    if (srcIndex < 0)
      return path;
    return path.slice(srcIndex + 1, path.length - 1);
  }

  function cppTemplateFor(document) {
    const path = sourcePath(document);
    const namespace = path[path.length - 1];
    const headerFileName = path.join('/') + '/' +
        document.name.replace('.cc', '.h');
    return copyrightTextCxx +
      `#include "${headerFileName}"\n\n` +
      `namespace ${namespace} {\n` +
      '\n' +
      `}  // namespace ${namespace}\n`;
  }

  function cheaderTemplateFor(document) {
    const path = sourcePath(document);
    const namespace = path[path.length - 1];
    const guard = (path.join('_') + '_' +
        document.name.replace('.', '_') + '_').toUpperCase();
    return copyrightTextCxx +
      `#ifndef ${guard}\n` +
      `#define ${guard}\n` +
      '\n' +
      `namespace ${namespace} {\n` +
      '\n' +
      `}  // namespace ${namespace}\n` +
      '\n' +
      `#endif  // ${guard}\n`;
  }

  text.TextDocumentTemplates.addTemplate('cc', cppTemplateFor);
  text.TextDocumentTemplates.addTemplate('gn', copyrightTextSharp);
  text.TextDocumentTemplates.addTemplate('h', cheaderTemplateFor);
  text.TextDocumentTemplates.addTemplate('js', copyrightTextCxx);
  text.TextDocumentTemplates.addTemplate('py', pythonTemplate);
})();
