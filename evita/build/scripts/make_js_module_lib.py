# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import struct
import sys

# JS Module Library format:
#
# Structure:
#   +-----------+
#   | Header    |
#   +-----------+
#   | Directory |
#   +-----------+
#   | Name pool |
#   +-----------+
#   | Blob pool |
#   +-----------+
#
# Detailed format:
#       +---------------------------+
#   +0  | number of modules(=N)     |
#       +----------------------------+
#   +4  | offset to entry[0].name   |
#       +---------------------------+
#   +8  | offset to entry[1].blob   |
#       +---------------------------+
#       ....
#       +---------------------------+
#       | offset to entry[N].name   | A sentinel entry to calculate length of
#       +---------------------------+ module name and blob for entry[N-1].
#       | offset to entry[N].blob   |
#       +---------------------------+
#       | Name pool                 |
#       +---------------------------+
#       | Blob pool                 |
#       +---------------------------+
#
#       * All numbers are 32-bit little endian
#       * All offsets are relative to start of library
#       * length of module name = entry[k+1].name_offset - entry[k].name_offset
#       * length of module blbo = entry[k+1].blob_offset - entry[k].blob_offset


def write_library_file_deprecated(output_file_name, module_map):
    module_names = sorted(module_map.keys())

    directory = []
    directory.append(struct.pack('<I', len(module_map)))
    name_offset = ((len(module_names) + 1) * 2 + 1) * 4
    module_offset = sum(len(module_name) for module_name in module_names) + \
        name_offset

    for module_name in module_names:
        directory.append(struct.pack('<I', name_offset))
        directory.append(struct.pack('<I', module_offset))
        name_offset += len(module_name)
        module_offset += len(module_map[module_name])
    directory.append(struct.pack('<I', name_offset))
    directory.append(struct.pack('<I', module_offset))

    with open(output_file_name, 'wb') as output_file:
        output_file.write(''.join(directory))
        output_file.write(''.join(module_names))
        for module_name in module_names:
            output_file.write(module_map[module_name])


# Simple string list version
def write_library_file(output_file_name, module_map):
    module_names = sorted(module_map.keys())
    directory = []
    directory.append(struct.pack('<I', len(module_map) * 2))
    string_offset = (len(module_names) * 2 + 2) * 4
    for module_name in module_names:
        directory.append(struct.pack('<I', string_offset))
        string_offset += len(module_name)
        directory.append(struct.pack('<I', string_offset))
        string_offset += len(module_map[module_name])
    directory.append(struct.pack('<I', string_offset))

    with open(output_file_name, 'wb') as output_file:
        output_file.write(''.join(directory))
        for module_name in module_names:
            output_file.write(module_name)
            output_file.write(module_map[module_name])


def main():
    if len(sys.argv) != 3:
        raise Exception('Usage: %s output_file list_file' %
                        os.path.basename(sys.argv[0]))
    output_file_name = sys.argv[1]
    list_file_name = sys.argv[2]

    with open(list_file_name, "rt") as list_file:
        file_names = list_file.read().split(' ')

    module_map = dict()
    for file_name in file_names:
        module_name, ext = os.path.splitext(os.path.basename(file_name))
        assert ext == '.jsobj', file_name
        with open(file_name, "rb") as module_file:
            module_map[module_name] = module_file.read()

    write_library_file(output_file_name, module_map)


if __name__ == '__main__':
    sys.exit(main())
