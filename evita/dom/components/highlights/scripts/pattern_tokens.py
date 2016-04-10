# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of self source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

INFINITY = float('inf')


def is_digit_char(char_code):
    return char_code >= ord('0') and char_code <= ord('9')


def is_space_char(char_code):
    return char_code == 0x20 or (char_code >= 0x09 and char_code <= 0x0D)


def is_word_char(char_code):
    if char_code >= ord('A') and char_code <= ord('Z'):
        return True
    if char_code >= ord('a') and char_code <= ord('z'):
        return True
    return char_code == ord('_') or is_digit_char(char_code)


def visible_char_of(char_code):
    if char_code == 0x22 or char_code == 0x27:
        return '\\%s' % chr(char_code)
    if char_code > 0x20 and char_code < 0x7F:
        return chr(char_code)
    return '\\u%04X' % char_code


class KnownSet(object):

    def __init__(self, kind):
        self._kind = kind

    @property
    def is_complement(self):
        return self._kind < ord('a')

    @property
    def is_digit(self):
        return self._kind == ord('d') or self._kind == ord('D')

    @property
    def is_range(self):
        return False

    @property
    def is_space(self):
        return self._kind == ord('s') or self._kind == ord('S')

    @property
    def is_word(self):
        return self._kind == ord('w') or self._kind == ord('W')

    def contains(self, char_code):
        if self._kind == ord('d'):
            return is_digit_char(char_code)
        if self._kind == ord('D'):
            return not is_digit_char(char_code)
        if self._kind == ord('s'):
            return is_space_char(char_code)
        if self._kind == ord('S'):
            return not is_space_char(char_code)
        if self._kind == ord('w'):
            return is_word_char(char_code)
        if self._kind == ord('W'):
            return not is_word_char(char_code)
        raise Exception('Invalid known set %s' % str(self))

    def is_singleton(self):
        return False

    def __str__(self):
        return '\\%s' % chr(self._kind)


class Range(object):

    def __init__(self, min_char_code, max_char_code):
        self._max_char_code = max_char_code
        self._min_char_code = min_char_code

    @property
    def is_range(self):
        return True

    @property
    def max_char_code(self):
        return self._max_char_code

    @property
    def min_char_code(self):
        return self._min_char_code

    def contains(self, char_code):
        return char_code >= self._min_char_code and \
            char_code <= self._max_char_code

    def is_singleton(self):
        return self._min_char_code == self._max_char_code

    def singleton(self):
        assert(self.is_singleton())
        return self._min_char_code

    def __str__(self):
        if self.min_char_code == self.max_char_code:
            return visible_char_of(self.min_char_code)
        return '%s-%s' % (visible_char_of(self.min_char_code),
                          visible_char_of(self.max_char_code))


class Token(object):

    def __init__(self, start, end):
        self._end = end
        self._start = start

    @property
    def is_any(self):
        return False

    @property
    def is_boundary(self):
        return False

    @property
    def is_char_set(self):
        return False

    @property
    def is_known_set(self):
        return False

    @property
    def is_lazy(self):
        return False

    @property
    def is_left_paren(self):
        return False

    @property
    def is_literal(self):
        return False

    @property
    def is_operator(self):
        return False

    @property
    def is_or(self):
        return False

    @property
    def is_primary(self):
        return False

    @property
    def is_quantifier(self):
        return False

    @property
    def is_right_paren(self):
        return False

    @property
    def name(self):
        raise Exception('Derived class should define "name" property')

    def __str__(self):
        return self.name


class PrimaryToken(Token):

    def __init__(self, start, end):
        super(PrimaryToken, self).__init__(start, end)

    @property
    def is_primary(self):
        return True


class AnyToken(PrimaryToken):

    def __init__(self, start, end):
        super(AnyToken, self).__init__(start, end)

    @property
    def is_any(self):
        return True

    @property
    def name(self):
        return '.'


class BoundaryToken(PrimaryToken):

    def __init__(self, start, end, kind):
        super(BoundaryToken, self).__init__(start, end)
        self._kind = kind

    @property
    def is_begin(self):
        return self._kind == ord('^')

    @property
    def is_boundary(self):
        return True

    @property
    def is_end(self):
        return self._kind == ord('$')

    @property
    def is_not_word_boundary(self):
        return self._kind == ord('B')

    @property
    def is_word_boundary(self):
        return self._kind == ord('b')

    @property
    def name(self):
        if self.is_begin:
            return '^'
        if self.is_end:
            return '$'
        return '\\%s' % chr(self._kind)


class CharSetToken(PrimaryToken):

    def __init__(self, start, end, is_complement, members):
        """|member| is either |KnownSet| or |Range|."""
        super(CharSetToken, self).__init__(start, end)
        self._is_complement = is_complement
        self._members = members

    @property
    def is_char_set(self):
        return True

    @property
    def is_complement(self):
        return self._is_complement

    @property
    def members(self):
        return self._members

    @property
    def name(self):
        members = [str(member) for member in self._members]
        if self._is_complement:
            return '[^%s]' % ''.join(members)
        return '[%s]' % ''.join(members)


class KnownSetToken(PrimaryToken):

    def __init__(self, start, end, kind):
        super(KnownSetToken, self).__init__(start, end)
        self._known_set = KnownSet(kind)

    @property
    def is_known_set(self):
        return True

    @property
    def known_set(self):
        return self._known_set

    @property
    def name(self):
        return str(self._known_set)


class LiteralToken(PrimaryToken):

    def __init__(self, start, end, char_code):
        super(LiteralToken, self).__init__(start, end)
        self._char_code = char_code

    @property
    def char_code(self):
        return self._char_code

    @property
    def is_literal(self):
        return True

    @property
    def name(self):
        return '"%s"' % visible_char_of(self._char_code)


class OperatorToken(Token):

    def __init__(self, start, end, kind):
        super(OperatorToken, self).__init__(start, end)
        self._kind = kind

    @property
    def is_left_paren(self):
        return self._kind == ord('(')

    @property
    def is_operator(self):
        return True

    @property
    def is_or(self):
        return self._kind == ord('|')

    @property
    def is_right_paren(self):
        return self._kind == ord(')')

    @property
    def kind(self):
        return self._kind

    @property
    def name(self):
        if self._kind == ord('b'):
            return '\\b'
        if self._kind == ord('B'):
            return '\\B'
        return chr(self._kind)


class QuantifierToken(Token):

    def __init__(self, start, end, min_count, max_count, is_lazy):
        super(QuantifierToken, self).__init__(start, end)
        assert(min_count >= 0)
        if is_lazy:
            assert(min_count < max_count)
        else:
            assert(min_count <= max_count)
        self._is_lazy = is_lazy
        self._min_count = min_count
        self._max_count = max_count

    @property
    def is_infinity(self):
        return self._max_count == INFINITY

    @property
    def is_lazy(self):
        return self._is_lazy

    @property
    def is_quantifier(self):
        return True

    @property
    def max_count(self):
        return self._max_count

    @property
    def min_count(self):
        return self._min_count

    @property
    def name(self):
        if self._min_count == 0 and self._max_count == INFINITY:
            return '*%s' % self._lazy_str()
        if self._min_count == 1 and self._max_count == INFINITY:
            return '+%s' % self._lazy_str()
        if self._min_count == 0 and self._max_count == 1:
            return '?%s' % self._lazy_str()
        if self._min_count == self._max_count:
            return '{%d}' % self._min_count
        if self._max_count == INFINITY:
            return '{%d,}%s' % (self._min_count, self._lazy_str())
        return '{%d,%d}%s' % (self._min_count, self._max_count,
                              self._lazy_str())

    def _lazy_str(self):
        return '?' if self._is_lazy else ''
