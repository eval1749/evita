# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class SourceLocation(object):
    def __init__(self, path, line, column):
        self._path = path
        self._line = line
        self._column = column

    @property
    def column(self):
        return self._column

    @property
    def line(self):
        return self._line

    @property
    def path(self):
        return self._path

    def __str__(self):
        return '%s(%d:%d)' % (self.path, self.line, self.column)


class PatternError(Exception):
    """A base class of pattern related exception."""
    def __init__(self, location, message):
        super(PatternError, self).__init__()
        self._location = location
        self._message = message

    @property
    def location(self):
        return self._location

    @property
    def message(self):
        return self._message

    def __str__(self):
        return '%s: %s' % (self._location, self._message)


class PatternLexerError(PatternError):
    """An exception raised when pattern lexical analyzer found source patter
    error."""

    def __init__(self, location, message):
        super(PatternLexerError, self).__init__(location, message)


class PatternParserError(PatternError):
    """An exception raised when pattern lexical analyzer found source patter
    error."""

    def __init__(self, location, message):
        super(PatternParserError, self).__init__(location, message)
