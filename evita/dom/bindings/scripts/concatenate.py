#!/usr/bin/python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Combine JS externs files into one file"""

import os
import re
import sys


def main(argv):
    if len(argv) != 3:
        raise Exception(
            'Usage: %s output_path list_files' % os.path.basename(argv[0]))

    output_path = argv[1]
    list_path = argv[2]

    with open(list_path) as list_file:
        file_names = list_file.read().split(' ')

    with open(output_path, 'wt') as output_file:
        for file_name in file_names:
            with open(file_name) as input_file:
                output_file.write('// %s\n' % file_name)
                output_file.write(input_file.read())


if __name__ == '__main__':
    sys.exit(main(sys.argv))
