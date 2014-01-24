#!/usr/bin/env python

import os, re, sys

script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(os.path.join(script_dir, os.pardir),
                            os.pardir))

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
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"

namespace dom {
namespace internal {

static const char ascii_script_source[] =
%(script_source)s;

const base::string16& GetJsLibSource() {
  CR_DEFINE_STATIC_LOCAL(base::string16, jslib_source,
                         (base::ASCIIToUTF16(ascii_script_source)));
  return jslib_source;
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

  script_source = 'var global = this;\n'
  for input_file in sys.argv[2:]:
    lines = '\n'.join(open(input_file, 'rt').readlines())
    script_source += lines

  script_source = RemoveCommentsAndTrailingWhitespace(script_source)
  script_source = minifier.JSMinify(script_source);
  if script_source[len(script_source) - 1] == '\n':
    script_source = script_source[0 : len(script_source) - 2]
  output = open(output_file, 'w');
  output.write(SOURCE % {
    'script_source': ToCString(script_source)
  })
  output.close()

if __name__ == '__main__':
  main()
