#!/usr/bin/env python

# Copyright 2015 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Fix include guard.
"""

import os
import re
import sys

class ProcessInfo(object):
  def __init__(self, file_name):
    self._file_name = file_name
    self._full_name = os.path.abspath(file_name).replace('\\', '/')
    root_name = FindRootDir(self._full_name)
    self._guard_name = GuardNameOf(self._full_name[len(root_name) + 1:])
    self._state = 'start'

  def Process(self, line):
    if self._state == 'start':
      if line.startswith('#if !defined(INCLUDE_'):
        self._state = 'ifndef'
        return '#ifndef ' + self._guard_name + '\n'
      if line.startswith('#ifndef'):
        return '#ifndef ' + self._guard_name + '\n'
      return line
    if self._state == 'ifndef':
      if line.startswith('#define INCLUDE_'):
        self._state = 'define'
        return '#define ' + self._guard_name + '\n'
    if self._state == 'define':
      if re.match(r'#endif *// *!defined[(]INCLUDE_', line):
        self._state = 'endif'
        return '#endif  // ' + self._guard_name + '\n'
    return line

def GuardNameOf(file_path_from_root):
  return re.sub(r'[^a-zA-Z0-9]', '_', file_path_from_root).upper() + '_'

def FindRootDir(full_name):
  dirname = os.path.dirname(full_name)
  while dirname != '/' and not HasRootMarker(dirname):
    dirname = os.path.dirname(dirname)
  return dirname


def HasRootMarker(dirname):
  return os.path.exists(os.path.join(dirname, '.git'))


def ProcessFile(file_name):
  file_info = ProcessInfo(file_name)
  lines = open(file_name, 'rt').readlines()
  state = 'start'
  modified = False
  new_lines = []
  for line in lines:
    new_line = file_info.Process(line)
    if new_line != line:
      modified = True
    new_lines.append(new_line)
  if not modified:
    return False
  new_lines.append('')
  sys.stderr.write('Update %s\n' % (file_name))
  open(file_name, 'wt').writelines(new_lines)
  return True


def main():
  for file_name in sys.argv[1:]:
    ProcessFile(file_name)

if __name__ == "__main__":
  main()
