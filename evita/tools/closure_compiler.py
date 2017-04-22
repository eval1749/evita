# Copyright (C) 2013 by Project Vogue.
# Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

import httplib
import urllib
import sys
import os
import re

# ${evita_src}/tools/closure_compiler.py
script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))

JAVA_OPTIONS = [
    '-Xms1G',
    '-Xbatch',
    '-Xnoclassgc',
    '-XX:+TieredCompilation',
]
CLOSURE_DIR = os.path.join(evita_src, 'third_party', 'closure_compiler')
CLOSURE_JAR = os.path.join(CLOSURE_DIR, 'closure-compiler-v20170409.jar')
ES6_MORE_EXTERNS_JS = os.path.join(CLOSURE_DIR, 'es6_more.js')

# See below list of warnings:
# https://code.google.com/p/closure-compiler/wiki/Warnings
CLOSURE_ERRORS = [
    'accessControls',
    'ambiguousFunctionDecl',
    'checkDebuggerStatement',
    'checkRegExp',
    'checkTypes',
    'checkVars',
    'const',
    'constantProperty',
    'deprecated',
    'externsValidation',
    'globalThis',
    'invalidCasts',
    'misplacedTypeAnnotation',
    'missingProperties',
    'missingReturn',
    'nonStandardJsDocs',
    'strictModuleDepCheck',
    'suspiciousCode',
    'undefinedNames',
    'undefinedVars',
    'unknownDefines',
    'unusedLocalVariables',
    'uselessCode',
    'visibility',
]

CLOSURE_WARNINGS = [
]

CLOSURE_OPTIONS = [
    '--checks-only',
    '--compilation_level=SIMPLE',
    '--formatting=PRETTY_PRINT',
    '--env=CUSTOM',
    '--language_in=ECMASCRIPT8',
    '--language_out=ECMASCRIPT5',
    '--summary_detail_level=3',
    '--warning_level=VERBOSE',
]


def makeOptions(name, values):
    if not len(values):
        return ''
    return name + ' ' + (' ' + name + ' ').join(values)


def run(js_output_file, js_files, js_externs, closure_options):
    params = {
        'java_options': ' '.join(JAVA_OPTIONS),
        'closure_errors': makeOptions('--jscomp_error', CLOSURE_ERRORS),
        'closure_warnings': makeOptions('--jscomp_warning', CLOSURE_WARNINGS),
        'closure_jar': CLOSURE_JAR,
        'closure_options': ' '.join(CLOSURE_OPTIONS + closure_options),
        'js_files': makeOptions('--js', js_files),
        'js_externs': makeOptions('--externs', js_externs),
    }
    command_line = ('java %(java_options)s -jar %(closure_jar)s' +
                    ' %(closure_options)s' +
                    ' %(closure_errors)s' +
                    ' %(closure_warnings)s' +
                    ' %(js_files)s' +
                    ' %(js_externs)s') % params
    exit_code = os.system(command_line)
    if exit_code == 0:
        with open(js_output_file, 'wt') as output:
            output.write('done')
    return exit_code


def readFile(filename):
    lines = ''.join(open(filename, 'rt').readlines())
    lines = re.sub(r"'use strict';", '// use strict', lines)
    return lines


def main():
    js_codes = []
    js_externs = [ES6_MORE_EXTERNS_JS]
    js_output_file = ''
    closure_options = []
    externs = None
    for arg in sys.argv[1:]:
        arg = re.sub(r'"', '', arg)
        if arg == '--extern':
            externs = True
        elif arg.startswith('--'):
            closure_options.append(arg)
            match = re.match(r'--js_output_file=(.+)$', arg)
            if match:
                js_output_file = re.sub(r'[\\]', '/', match.group(1))
        elif externs:
            js_externs.append(arg)
        else:
            js_codes.append(arg)
    return run(js_output_file, js_codes, js_externs, closure_options)

if __name__ == '__main__':
    sys.exit(main())
