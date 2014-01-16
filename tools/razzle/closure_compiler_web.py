# Copyright (C) 2013 by Project Vogue.
# Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

import httplib, urllib, sys
import re;

# Define the parameters for the POST request and encode them in
# a URL-safe format.
def post(js_codes, js_externs):
  params = [
      ('exclude_default_externs', 'true'),
      ('compilation_level', 'ADVANCED_OPTIMIZATIONS'),
      ('output_format', 'text'),
      ('output_info', 'errors'),
      ('output_info', 'errors'),
      ('output_info', 'warnings'),
      ('output_info', 'statistics'),
      ('output_info', 'compiled_code'),
      ('warning_level', 'VERBOSE'),
      ('language', 'ECMASCRIPT5_STRICT'),
  ]
  params.extend(map(lambda x: ('js_code', x), js_codes))
  params.extend(map(lambda x: ('js_externs', x), js_externs))
  post_data = urllib.urlencode(params)

  # Always use the following value for the Content-type header.
  headers = { "Content-type": "application/x-www-form-urlencoded" }
  conn = httplib.HTTPConnection('closure-compiler.appspot.com')
  conn.request('POST', '/compile', post_data, headers)
  response = conn.getresponse()
  data = response.read()
  print data
  conn.close()

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
      js_externs.append(readFile(arg))
    else:
      js_codes.append(readFile(arg))
  post(js_codes, js_externs)

if __name__ == '__main__':
  main()
