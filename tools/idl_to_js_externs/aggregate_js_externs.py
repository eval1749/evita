#!/usr/bin/python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Combine JS externs files into one file"""

import os
import re
import sys


def main(argv):
    if len(argv) != 7:
        raise Exception(
            'Usage: %s output_path header_js_path js_externs_directory idl_list_path static_directory static_list_path' % os.path.basename(argv[0]))

    _, output_path, header_js_path, js_externs_dir, idl_list_path, static_dir, static_list_path = argv

    with open(header_js_path) as header_js_file:
        header_js_text = header_js_file.read()

    with open(idl_list_path) as idl_list_file:
        idl_file_names = [line.rstrip() for line in idl_list_file]

    names = sorted([os.path.splitext(os.path.basename(idl_file_name))[0] for
                    idl_file_name in idl_file_names])

    with open(static_list_path) as static_list_file:
        static_file_names = [line.rstrip() for line in static_list_file]

    with open(output_path, 'wt') as output_file:
        output_file.write(header_js_text)

        for name in names:
            js_externs_path = os.path.join(js_externs_dir,
                                           name + '_externs.js')
            with open(js_externs_path) as js_externs_file:
                output_file.write(js_externs_file.read())

        for static_file_name in static_file_names:
            if static_file_name[1] == ':':
                static_path = static_file_name
            else:
                static_path = os.path.join(static_dir, static_file_name)
            with open(static_path) as static_file:
                output_file.write(static_file.read())


if __name__ == '__main__':
    sys.exit(main(sys.argv))
