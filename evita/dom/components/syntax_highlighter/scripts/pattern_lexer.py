# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of self source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

from pattern import PatternLexerError
from pattern_tokens import AnyToken, BoundaryToken, CharSetToken, KnownSet, \
                           KnownSetToken, LiteralToken, OperatorToken, \
                           QuantifierToken, Range, INFINITY
from string_reader import StringReader

BACKSLASH = 0x5C
LEFT_BRACKET = 0x5B
LEFT_PAREN = 0x28
RIGHT_BRACKET = 0x5D
RIGHT_PAREN = 0x29

ESCAPE_CHAR_MAP = {
    ord('b'): 8,
    ord('f'): 12,
    ord('n'): 10,
    ord('r'): 13,
    ord('t'): 9,
    ord('v'): 11,
    BACKSLASH: BACKSLASH,
}

KNOWN_SET_CHARS = set([
    ord('d'), ord('D'),
    ord('s'), ord('S'),
    ord('w'), ord('W'),
])

META_CHARS = set([
    ord('('), ord(')'),
    ord('['), ord(']'),
    ord('{'), ord('}'),
    ord('+'), ord('*'), ord('?'),
    ord('|'),
    ord('.'),
])

class PatternLexer(object):
    def __init__(self, characters):
        # |_index| holds number of characters processed so far.
        self._index = 0
        self._location = None
        self._reader = StringReader('-', characters)
        self._token = None

    @property
    def is_eof(self):
        return self._token == None and self._reader.is_eof

    @property
    def location(self):
        return self._reader.location

    def _add_member(self, members, range_start, range_end):
        if range_start >= range_end:
            return self.error('Bad range %c-%c' % (chr(range_start),
                                                   chr(range_end)))
        members.append(Range(range_start, range_end))

    def advance(self):
        self._token = None
        if self._reader.is_eof:
            return False
        state = 'start'
        is_complement = False
        members = []
        range_start = -1
        self._location = self.location
        while not self._reader.is_eof:
            char_code = self._reader.read()
            self._index = self._index + 1
            if state == 'start':
                if char_code == ord('*'):
                    state = 'asterisk'
                    continue
                if char_code == ord('+'):
                    state = 'plus'
                    continue
                if char_code == ord('?'):
                    state = 'question'
                    continue
                if char_code == LEFT_BRACKET:
                    state = 'bracket'
                    continue
                if char_code == BACKSLASH:
                    state = 'backslash'
                    continue
                if char_code == ord('^') and self._index == 1:
                    return self._remember_boundary(char_code)
                if char_code == ord('$') and self.is_eof:
                    return self._remember_boundary(char_code)
                if char_code == ord('.'):
                    return self._remember_any()
                if char_code in META_CHARS:
                    return self._remember_operator(char_code)
                return self._remember_literal(char_code)

            if state == 'asterisk':
                if char_code == ord('?'):
                    return self._remember_quantifier(0, INFINITY, True)
                self._reader.unread()
                return self._remember_quantifier(0, INFINITY, False)

            if state == 'backslash':
                if char_code == ord('b') or char_code == ord('B'):
                    return self._remember_boundary(char_code)
                if char_code in KNOWN_SET_CHARS:
                    return self._remember_known_set(char_code)
                if char_code in META_CHARS:
                    return self._remember_literal(char_code)
                # Since '\b' denotes word boundary rather than U+0008. To
                # specify U+0008, we should use '[\b]' or '\u0008'.
                if char_code in ESCAPE_CHAR_MAP:
                    return self._remember_literal(ESCAPE_CHAR_MAP[char_code])
                return self.error('Bad escape %c' % chr(char_code))

            if state == 'plus':
                if char_code == ord('?'):
                    return self._remember_quantifier(1, INFINITY, True)
                self._reader.unread()
                return self._remember_quantifier(1, INFINITY, False)

            if state == 'question':
                if char_code == ord('?'):
                    return self._remember_quantifier(0, 1, True)
                self._reader.unread()
                return self._remember_quantifier(0, 1, False)

            if state == 'bracket':
                members = []
                is_complement = char_code == ord('^')
                if is_complement:
                    state = 'bracket_body'
                    continue
                if char_code == BACKSLASH:
                    state = 'bracket_backslash'
                    continue
                range_start = char_code
                state = 'bracket_start'
                continue

            if state == 'bracket_backslash':
                if char_code in ESCAPE_CHAR_MAP:
                    range_start = ESCAPE_CHAR_MAP[char_code]
                    state = 'bracket_start'
                    continue
                if char_code in KNOWN_SET_CHARS:
                    members.append(KnownSet(char_code))
                    state = 'bracket_body'
                    continue
                if char_code == ord('-') or char_code == RIGHT_BRACKET:
                    range_start = char_code
                    state = 'bracket_start'
                    continue
                return self.error('Bad backslash %c' % chr(char_code))

            if state == 'bracket_body':
                if char_code == BACKSLASH:
                    state = 'bracket_backslash'
                    continue
                if char_code == RIGHT_BRACKET:
                    assert(len(members) >= 1)
                    return self._remember_char_set(is_complement, members)
                range_start = char_code
                state = 'bracket_start'
                continue

            if state == 'bracket_start':
                if char_code == ord('-'):
                    state = 'bracket_range'
                    continue
                members.append(Range(range_start, range_start))
                if char_code == RIGHT_BRACKET:
                    return self._remember_char_set(is_complement, members)
                if char_code == BACKSLASH:
                    state = 'bracket_backslash'
                    continue
                range_start = char_code
                state = 'bracket_start'
                continue

            if state == 'bracket_range':
                if char_code == BACKSLASH:
                    state = 'bracket_range_backslash'
                    continue
                self._add_member(members, range_start, char_code)
                state = 'bracket_body'
                continue

            if state == 'bracket_range_backslash':
                if char_code in ESCAPE_CHAR_MAP:
                    self._add_member(members, char_code,
                                     ESCAPE_CHAR_MAP[char_code])
                    state = 'bracket_body'
                    continue
                return self.error('Bad range %c-\\%c', (chr(range_start),
                                                        chr(char_code)))

            raise Exception('Bad state %s' % state)

        if state == 'start':
            return True
        if state == 'asterisk':
            return self._remember_quantifier(0, INFINITY, False)
        if state == 'plus':
            return self._remember_quantifier(1, INFINITY, False)
        if state == 'question':
            return self._remember_quantifier(0, 1, False)
        self._reader.error('Unexpected EOF state=%s' % state)
        return True

    def error(self, message):
        raise PatternLexerError(self.location, message)

    def peek(self):
        if self._token == None:
            self.error('You should call advance()')
        return self._token

    def _remember_any(self):
        self._token = AnyToken(self._location, self.location)
        return True

    def _remember_boundary(self, char_code):
        self._token = BoundaryToken(self._location, self.location, char_code)
        return True

    def _remember_char_set(self, is_complement, members):
        if len(members) == 1 and not is_complement and \
           members[0].is_singleton():
            return self._remember_literal(members[0].singleton())
        self._token = CharSetToken(self._location, self.location,
                                   is_complement, members)
        return True

    def _remember_known_set(self, char_code):
        self._token = KnownSetToken(self._location, self.location, char_code)
        return True

    def _remember_literal(self, char_code):
        self._token = LiteralToken(self._location, self.location, char_code)
        return True

    def _remember_operator(self, kind):
        self._token = OperatorToken(self._location, self.location, kind)
        return True

    def _remember_quantifier(self, min_count, max_count, is_lazy):
        self._token = QuantifierToken(self._location, self.location, min_count,
                                      max_count, is_lazy)
        return True
