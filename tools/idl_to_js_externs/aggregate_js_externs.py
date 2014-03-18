#!/usr/bin/python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Combine JS externs files into one file"""

import os
import re
import sys
from textwrap import TextWrapper


def main(argv):
    if len(argv) != 5:
        raise Exception('Usage: %s output_path header_js_path js_externs_directory idl_list_path' % os.path.basename(argv[0]))

    _, output_path, header_js_path, js_externs_dir, idl_list_path = argv

    with open(header_js_path) as header_js_file:
        header_js_text = header_js_file.read()

    with open(idl_list_path) as idl_list_file:
        idl_files = [line.rstrip() for line in idl_list_file]

    names = sorted([os.path.splitext(os.path.basename(idl_file))[0]
                    for idl_file in idl_files])

    with open(output_path, 'wt') as output_file:
        output_file.write(header_js_text)

        wrapper = TextWrapper(subsequent_indent=' *  ', width=78)
        output_file.write("""
/*
 * This file contains the following classes:
 *  %s
 */

""" % '\n'.join(wrapper.wrap(' '.join(names))))

        for name in names:
            js_externs_path = os.path.join(js_externs_dir, name + '_externs.js')
            with open(js_externs_path) as js_externs_file:
                output_file.write(js_externs_file.read())

if __name__ == '__main__':
    sys.exit(main(sys.argv))
