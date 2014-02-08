#!/usr/bin/env python

import os, re, sys

script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))

sys.path.insert(0, os.path.join(evita_src, 'v8', 'tools'))
import jsmin

SOURCE = """\
// L4 C4127: conditional expression is constant
// L1 C4350: behavior change: 'member1' called instead of 'member2'
// L4 C4365: 'action' : conversion from 'type_1' to 'type_2', signed/unsigned
// mismatch
// L1 C4530: C++ exception handler used, but unwind semantics are not enabled.
// Specify /EHsc
#pragma warning(disable: 4127 4350 4365 4530)
#include "base/basictypes.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/v8_glue/runner.h"

namespace dom {
namespace internal {

namespace {
struct ScriptSouce {
  const char* file_name;
  const char* script_text;
};
const ScriptSouce entries[] = {
%(entries)s
};
}  // namespace

bool GetJsLibFiles(v8_glue::Runner* runner) {
  for (auto index = 0u; index < arraysize(entries); ++index) {
    auto entry = &entries[index];
    auto result = runner->Run(
        base::ASCIIToUTF16(entry->script_text),
        base::ASCIIToUTF16(entry->file_name));
    if (result.IsEmpty())
      return false;
  }
  return true;
}

}  // namespace internal
}  // namespace dom
"""

kMaxCharsInLine = 80
kIndent = '  '

def RemoveCommentsAndTrailingWhitespace(lines):
  lines = re.sub(r'\s*//.*\n', '\n', lines) # end-of-line comments
  lines = re.sub(re.compile(r'/\*.*?\*/', re.DOTALL), '', lines) # comments.
  return lines

def ToCString(lines):
  result = ''
  line = kIndent
  for line in lines.split('\n'):
    line = re.sub(r'^\s+', '', line)
    line = re.sub(r'^s+$', '', line)
    if line == '':
      continue
    if len(result):
      result += '\n'
    line = re.sub(r'\\', '\\\\\\\\', line)
    line = re.sub(r'"', '\\"', line)
    result += kIndent + '"' + line + '\\n"'
  return result;

def main():
  output_file = sys.argv[1]
  minifier = jsmin.JavaScriptMinifier()

  entries = []

  entries.append('{"(global)", "var global = this;"},');

  for input_file in sys.argv[2:]:
    script_text = '\n'.join(open(input_file, 'rt').readlines())
    script_text = RemoveCommentsAndTrailingWhitespace(script_text)
    script_text = minifier.JSMinify(script_text);
    entries.append('{"%(file_name)s", %(script_text)s},' % {
      'file_name': input_file,
      'script_text': ToCString(script_text)
    })

  output = open(output_file, 'w');
  output.write(SOURCE % {
    'entries': '\n'.join(entries)
  })
  output.close()

if __name__ == '__main__':
  main()
