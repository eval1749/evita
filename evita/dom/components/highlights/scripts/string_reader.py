# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from pattern import SourceLocation

NEWLINE = 10


class StringReader(object):

    def __init__(self, input_path, characters):
        self._characters = characters
        self._column_number = 0
        self._index = 0
        self._line_number = 1
        self._input_path = input_path

    @property
    def is_eof(self):
        return self._index == len(self._characters)

    @property
    def location(self):
        return SourceLocation(self._input_path, self._line_number,
                              self._column_number)

    def error(self, message):
        raise Exception('%s: %s' % (self.location, message))

    def read(self):
        if self.is_eof:
            self.error('Unexpected EOF')
        char_code = ord(self._characters[self._index])
        self._index = self._index + 1
        if char_code == NEWLINE:
            self._line_number = self._line_number + 1
            self._column_number = 0
        else:
            self._column_number = self._column_number + 1
        return char_code

    def unread(self):
        if self._index == 0:
            self.error('Can not unread')
        self._index = self._index - 1
        char_code = ord(self._characters[self._index])
        if char_code == NEWLINE:
            self._line_number = self._line_number - 1
            self._column_number = 0
        else:
            self._column_number = self._column_number - 1
