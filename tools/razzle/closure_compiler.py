# Copyright (C) 2013 by Project Vogue.
# Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

import httplib, urllib, sys
import os
import re

# ${evita_src}/tools/closure_compiler.py
script_dir = os.path.dirname(os.path.realpath(__file__))
evita_src = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir));

JAVA_OPTIONS = ['-d64', '-server'];
CLOSURE_JAR = os.path.join(evita_src, 'third_party', 'closure_compiler',
                          'compiler.jar');

# See below folow list of warnings:
# https://code.google.com/p/closure-compiler/wiki/Warnings
CLOSURE_ERRORS = [
  'accessControls',
  'checkRegExp',
  'checkDebuggerStatement',
  'const',
  'constantProperty',
  'strictModuleDepCheck',
  'visibility',
];

CLOSURE_WARNINGS = [
];

CLOSURE_OPTIONS = [
  #'--formatting=PRETTY_PRINT',
  '--js_output_file=nul',
  '--language_in=ECMASCRIPT5_STRICT',
  '--summary_detail_level=3',
  '--use_only_custom_externs',
  '--warning_level=VERBOSE',
];

def makeOptions(name, values):
  if not len(values):
    return ''
  return name + ' ' + (' ' + name + ' ').join(values)

def run(js_files, js_externs):
  params = {
    'java_options': ' '.join(JAVA_OPTIONS),
    'closure_errors': makeOptions('--jscomp_error', CLOSURE_ERRORS),
    'closure_warnings': makeOptions('--jscomp_warning', CLOSURE_WARNINGS),
    'closure_jar': CLOSURE_JAR,
    'closure_options': ' '.join(CLOSURE_OPTIONS),
    'js_files': makeOptions('--js', js_files),
    'js_externs': makeOptions('--externs', js_externs),
  }
  command_line = ('java %(java_options)s -jar %(closure_jar)s' + \
                  ' %(closure_options)s' + \
                  ' %(closure_errors)s' + \
                  ' %(closure_warnings)s' + \
                  ' %(js_files)s' + \
                  ' %(js_externs)s') % params;
  print command_line
  os.system(command_line)

def readFile(filename):
  lines = ''.join(open(filename, 'rt').readlines());
  lines = re.sub(r"'use strict';", '// use strict', lines);
  return lines;

def main():
  js_codes = [];
  js_externs = [];
  externs = None
  for arg in sys.argv[1:]:
    if arg == '--extern':
      externs = True;
    elif externs:
      js_externs.append(arg)
    else:
      js_codes.append(arg)
  run(js_codes, js_externs)

if __name__ == '__main__':
  main()
