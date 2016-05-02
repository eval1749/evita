# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import string
import struct
import sys

# JS Module binary file
#   +-------------------+
#   | Number of strings |
#   +-------------------+
#   | Offset Table      |
#   +-------------------+
#   | String Pool       |
#   +-------------------+
#
# Offset table
#   N = <number of entries : uint32>
#   <offset, from stable start, to string data : uint32> x (N + 1)
#   Length of string[k] is offset[k+1] - offset[k]
#
# String Pool
#   N = <number of characters : uint32>
#   <characters : uint8> x N


def minify(script_text):
    result = script_text
    result = re.sub(r'/[*][^\n]+?[*]/', '', result)
    result = re.sub(r'/[*].+?[*]/', minify_block_comment, result,
                    flags=re.DOTALL)
    result = re.sub(r'\n +', '\n', result)
    result = re.sub(r'//.*?\n', '\n', result)
    return result


def minify_block_comment(match_obj):
    return '\n' * string.count(match_obj.group(0), '\n')


def read_file(file_name):
    with open(file_name, 'rb') as js_file:
        return ''.join(js_file.readlines())


def write_to_file(output_file_name, data_strings):
    offsets = []
    string_pool = []
    string_offset = (len(data_strings) + 2) * 4
    offsets.append(struct.pack('<I', len(data_strings)))
    for data_string in data_strings:
        offsets.append(struct.pack('<I', string_offset))
        string_pool.append(data_string)
        string_offset += len(data_string)
    offsets.append(struct.pack('<I', string_offset))

    with open(output_file_name, 'wb') as output_file:
        output_file.write(''.join(offsets))
        output_file.write(''.join(string_pool))


def main():
    if len(sys.argv) != 3:
        raise Exception('Usage: %s output_file list_file' %
                        os.path.basename(sys.argv[0]))

    output_file_name = sys.argv[1]
    list_file_name = sys.argv[2]

    file_names = []
    with open(list_file_name) as list_file:
        file_names = list_file.read().split(' ')

    strings = []
    for file_name in file_names:
        strings.append(os.path.basename(file_name))
        script = read_file(file_name)
        strings.append(minify(script))

    write_to_file(output_file_name, strings)


if __name__ == '__main__':
    sys.exit(main())
