#!/usr/bin/env python

import os
import re
import sys

script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))

STRINGS_H = """
// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_v8_strings_h)
#define INCLUDE_evita_dom_v8_strings_h

#include <new>

#include "evita/v8_glue/v8.h"

namespace dom {
namespace v8Strings {

%(externs)s

void Init(v8::Isolate* isolate);

}  // namespace v8Strings
}  // namespace dom

#endif // !defined(INCLUDE_evita_dom_v8_strings_h)
"""

STRINGS_CC = """
#include "evita/dom/v8_strings.h"

namespace dom {
namespace v8Strings {

// L1 C4075 initializers put in unrecognized initialization area
#pragma warning(disable: 4075)
#pragma init_seg(".unwantedstaticinits")

typedef v8::Eternal<v8::String> V8String;

%(defs)s

static v8::Local<v8::String> NewString(v8::Isolate* isolate,
                                        const char* string) {
  return v8::String::NewFromOneByte(isolate,
                                    reinterpret_cast<const uint8_t*>(string));
}

void Init(v8::Isolate* isolate) {
%(inits)s
}

}  // namespace v8Strings
}  // namespace dom
"""

DEF = 'V8String %(name)s;'
EXTERN = 'extern v8::Eternal<v8::String> %(name)s;'
INIT = '  new(&%(name)s) V8String(isolate, NewString(isolate, "%(name)s"));'


def chop(line):
    return line[0: len(line) - 1]


def exclude(line):
    if len(line) == 0:
        return None
    if line.startswith('#'):
        return None
    return True


def generate(output_prefix, input_file):
    lines = open(input_file, 'rt').readlines()
    name_set = frozenset(filter(exclude, map(chop, lines)))
    names = sorted(name_set)

    defs = '\n'.join(map(lambda name: DEF % {'name': name}, names))
    externs = '\n'.join(map(lambda name: EXTERN % {'name': name}, names))
    inits = '\n'.join(map(lambda name: INIT % {'name': name}, names))

    header_output = open(output_prefix + '.h', 'w')
    header_output.write(STRINGS_H % {'externs': externs})
    header_output.close()

    cc_output = open(output_prefix + '.cc', 'w')
    cc_output.write(STRINGS_CC % {'defs': defs, 'inits': inits})
    cc_output.close()


def main():
    output_prefix = sys.argv[1]
    input_file = sys.argv[2]
    generate(output_prefix, input_file)

if __name__ == '__main__':
    main()
