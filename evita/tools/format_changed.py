# Copyright (c) 2015 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import pipes
import sys

FILE_TYPES = {
    '.cc': 'c++',
    '.h': 'c++',
    '.cpp': 'c++',
    '.c': 'c++',
    '.gn': 'gn',
    '.gni': 'gni',
    '.js': 'js',
    '.java': 'java',
}


def file_type_of(file_name):
    for (extension, type_name) in FILE_TYPES.iteritems():
        if file_name.endswith(extension):
            return type_name
    return None


def dispatch(file_name):
    if file_name.find('/templates/') >= 0:
        return skip(file_name)

    file_type = file_type_of(file_name)
    if file_type == 'c++':
        return format_cpp(file_name)
    if file_type == 'gn':
        return format_gn(file_name)
    return None


def format_cpp(file_name):
    sys.stderr.write('  Formatting C++ "%s"\n' % file_name)
    os.system('clang-format -i %s' % file_name)


def format_gn(file_name):
    """Format with 'gn format' command. On Windows, output lines end with CRLF.
    """
    os.system('gn format --in-place %s' % file_name)


def skip(file_name):
    sys.stderr.write('Skip "%s"\n' % file_name)


def main():
    git_pipe = pipes.Template()
    git_pipe.prepend('git diff master --name-status', '.-')
    diff_output = git_pipe.open('files', 'r')
    lines = diff_output.readlines()
    diff_output.close()
    for line in lines:
        line = line.rstrip()
        if len(line) == 0:
            continue
        words = line.split()
        if words[0] == 'D':
            continue
        dispatch(words[1])

if __name__ == '__main__':
    sys.exit(main())
