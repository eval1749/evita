#!/usr/bin/env python

import os
import re
import sys
import string

script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))

#sys.path.insert(0, os.path.join(evita_src, 'v8', 'tools'))
#import jsmin

SOURCE = """\
// L4 C4127: conditional expression is constant
// L1 C4350: behavior change: 'member1' called instead of 'member2'
// L4 C4365: 'action' : conversion from 'type_1' to 'type_2', signed/unsigned
// mismatch
// L1 C4530: C++ exception handler used, but unwind semantics are not enabled.
// Specify /EHsc
#pragma warning(disable: 4127 4350 4365 4530)
#include "base/macros.h"
#include "evita/dom/static_script_source.h"

namespace dom {
namespace internal {

namespace {
const StaticScriptSource entries[] = {
%(entries)s
};
}  // namespace

const std::vector<StaticScriptSource>& GetJsLibSources() {
  CR_DEFINE_STATIC_LOCAL(std::vector<StaticScriptSource>, sources, ());
  if (sources.empty()) {
    for (auto index = 0u; index < arraysize(entries); ++index) {
      sources.push_back(entries[index]);
    }
  }
  return sources;
}

}  // namespace internal
}  // namespace dom
"""

kMaxCharsInLine = 80
kIndent = '  '


def ConvertToCString(script_text):
    lines = script_text.split('\n')
    result = ''
    for line in lines:
        line = re.sub(r'\\', '\\\\\\\\', line)
        line = re.sub(r'"', '\\"', line)
        result += kIndent + '"' + line + '\\n"' + '\n'
    return result


def handleMultipleLineComment(match_obj):
    return '\n' * string.count(match_obj.group(0), '\n')


def minify(script_text):
    result = script_text
    result = re.sub(r'/[*][^\n]+?[*]/', '', result)
    result = re.sub(r'/[*].+?[*]/', handleMultipleLineComment, result,
                    flags=re.DOTALL)
    result = re.sub(r'\n +', '\n', result)
    result = re.sub(r'//.*?\n', '\n', result)
    return result


def main():
    output_file = sys.argv[1]
    #minifier = jsmin.JavaScriptMinifier()

    entries = []

    entries.append('{"(global)", "var global = this;"},')

    for input_file in sys.argv[2:]:
        lines = open(input_file, 'rt').readlines()
        script_text = ''.join(lines)
        # script_text = minifier.JSMinify(script_text)
        script_text = minify(script_text)
        entries.append('{"%(file_name)s", \n%(script_text)s},' % {
            'file_name': input_file,
            'script_text': ConvertToCString(script_text)
        })

    output = open(output_file, 'w')
    output.write(SOURCE % {
        'entries': '\n'.join(entries)
    })
    output.close()

if __name__ == '__main__':
    main()
