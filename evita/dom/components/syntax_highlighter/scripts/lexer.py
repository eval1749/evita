# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from tokens import ActionToken, CharToken, LabelToken, NameToken, OperatorToken
from string_reader import StringReader

BACKSLASH = ord('\\')
CAPITAL_LETTER_A = ord('A')
CAPITAL_LETTER_Z = ord('Z')
COLON = ord(':')
DIGIT_ZERO = ord('0')
DIGIT_NINE = ord('9')
LPAREN = ord('(')
NUMBER_SIGN = ord('#')
NEWLINE = ord('\n')
RPAREN = ord(')')
SINGLE_QUOTE = ord('\'')
SMALL_LETTER_A = ord('a')
SMALL_LETTER_Z = ord('z')
UNDERLINE = ord('_')


class Lexer(object):
    def __init__(self, input_path):
        self._state = 'start'
        with open(input_path, 'r') as content_file:
            self._reader = StringReader(input_path, content_file.read())

    @property
    def location(self):
        return self._read.location

    def error(self, message):
        return self._reader.error(message)

    def next_token(self):
        single_quote = 0
        name_accumulator = []
        state = 'start'
        start_location = self.location
        while True:
            if self._reader.is_eof:
                return None
            char_code = self._reader.read()
            if state == 'start':
                if char_code == NUMBER_SIGN:
                    state = 'comment'
                    continue
                if char_code == SINGLE_QUOTE:
                    state = 'single_quote'
                    continue
                if is_name_start_char(char_code):
                    state = 'name'
                    name_accumulator = [chr(char_code)]
                    continue
                if is_whitespace(char_code):
                    start_location = self.location
                    continue
                return OperatorToken(char_code, start_location, self.location)

            if state == 'comment':
                if char_code == NEWLINE:
                    state = 'start'
                continue

            if state == 'name':
                if is_name_char(char_code):
                    name_accumulator.append(chr(char_code))
                    continue
                end_location = self.location
                if char_code == COLON:
                    return LabelToken(''.join(name_accumulator), start_location,
                                      end_location)
                if char_code == LPAREN:
                    return ActionToken(''.join(name_accumulator),
                                       start_location, end_location)
                self.unread()
                return NameToken(''.join(name_accumulator), start_location,
                                 end_location)

            if state == 'single_quote':
                if char_code == BACKSLASH:
                    state = 'single_quote_escape'
                    continue
                single_quote = char_code
                state = 'single_quote_char'
                continue

            if state == 'single_quote_escape':
                single_quote = char_code
                state = 'single_quote_char'
                continue

            if state == 'single_quote_char':
                return CharToken(single_quote, start_location,
                                 self.location)

            self.error('Bad state %s' % state)


class Location(object):
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

def is_digit_char(char_code):
    return char_code >= DIGIT_ZERO and char_code <= DIGIT_NINE


def is_name_char(char_code):
    return is_name_start_char(char_code) or is_digit_char(char_code);


def is_name_start_char(char_code):
    return char_code >= CAPITAL_LETTER_A and char_code <= CAPITAL_LETTER_Z or \
           char_code >= SMALL_LETTER_A and char_code <= SMALL_LETTER_Z or \
           char_code == UNDERLINE


def is_whitespace(char_code):
    return char_code <= 0x20
