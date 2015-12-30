# Copyright (c) 2015 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A wrapper script for using autopep8 from the command line."""

import os
import subprocess
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.normpath(os.path.join(_HERE, os.pardir, os.pardir))
_AUTOPEP8 = os.path.join(_ROOT, 'third_party', 'autopep8', 'autopep8.py')
_PEP8 = os.path.join(_ROOT, 'third_party', 'pep8')


def main():
    if not ('PYTHONPATH' in os.environ):
        os.environ['PYTHONPATH'] = ''
    os.environ['PYTHONPATH'] += _PEP8 + os.pathsep
    command = [sys.executable, _AUTOPEP8]
    command.extend(sys.argv[1:])
    try:
        sys.exit(subprocess.call(command))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)

if __name__ == '__main__':
    sys.exit(main())
