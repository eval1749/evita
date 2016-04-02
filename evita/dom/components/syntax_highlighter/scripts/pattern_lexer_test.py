# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import pattern
from pattern_lexer import PatternLexer

def tokens_of(source):
    lexer = PatternLexer(source)
    tokens = []
    while lexer.advance():
        tokens.append(str(lexer.peek()))
    return ' '.join(tokens)


class PatternLexerTest(unittest.TestCase):
    def test_basics(self):
        self.assertEqual('"a" "b" "c"', tokens_of('abc'))
        self.assertEqual('.', tokens_of('.'))
        self.assertEqual('*', tokens_of('*'))
        self.assertEqual('*?', tokens_of('*?'))
        self.assertEqual('+', tokens_of('+'))
        self.assertEqual('+?', tokens_of('+?'))
        self.assertEqual('?', tokens_of('?'))
        self.assertEqual('??', tokens_of('??'))
        self.assertEqual('|', tokens_of('|'))
        self.assertEqual('.', tokens_of('.'))
        self.assertEqual('( "a" )', tokens_of('(a)'))

    def test_backslashes(self):
        self.assertEqual('"\\u000C"', tokens_of('\\f'))
        self.assertEqual('"\\u000A"', tokens_of('\\n'))
        self.assertEqual('"\\u000D"', tokens_of('\\r'))
        self.assertEqual('"\\u000B"', tokens_of('\\v'))
        self.assertEqual('"\\"', tokens_of('\\\\'))

        self.assertEqual('\\b', tokens_of('\\b'))
        self.assertEqual('\\B', tokens_of('\\B'))
        self.assertEqual('\\d', tokens_of('\\d'))
        self.assertEqual('\\D', tokens_of('\\D'))
        self.assertEqual('\\w', tokens_of('\\w'))
        self.assertEqual('\\W', tokens_of('\\W'))

        self.assertEqual('"*"', tokens_of('\\*'))
        self.assertEqual('"+"', tokens_of('\\+'))
        self.assertEqual('"?"', tokens_of('\\?'))
        self.assertEqual('"("', tokens_of('\\('))
        self.assertEqual('")"', tokens_of('\\)'))
        self.assertEqual('"["', tokens_of('\\['))
        self.assertEqual('"]"', tokens_of('\\]'))
        self.assertEqual('"{"', tokens_of('\\{'))
        self.assertEqual('"}"', tokens_of('\\}'))
        self.assertEqual('"|"', tokens_of('\\|'))
        self.assertEqual('"."', tokens_of('\\.'))

    def test_boundary(self):
        self.assertEqual('^', tokens_of('^'))
        self.assertEqual('$', tokens_of('$'))

        # "^" is a meta char if and only if it appears at start of source
        self.assertEqual('"a" "^"', tokens_of('a^'))
        # "$" is a meta char if and only if it appears at end of source
        self.assertEqual('"$" "a"', tokens_of('$a'))

    def test_error(self):
        with self.assertRaisesRegexp(Exception, 'Unexpected EOF'):
            tokens_of('\\')
        with self.assertRaisesRegexp(Exception, 'Unexpected EOF'):
            tokens_of('[a')
        with self.assertRaisesRegexp(pattern.PatternLexerError, 'Bad escape q'):
            tokens_of('\\q')
        with self.assertRaisesRegexp(pattern.PatternLexerError,
                                     'Bad backslash q'):
            tokens_of('[\\q]')
        with self.assertRaisesRegexp(pattern.PatternLexerError,
                                     'Bad range a-a'):
            tokens_of('[a-a]')
        with self.assertRaisesRegexp(pattern.PatternLexerError,
                                     'Bad range z-a'):
            tokens_of('[z-a]')

    def test_sets(self):
        self.assertEqual('"a"', tokens_of('[a]'), msg='singleton')
        self.assertEqual('[^a]', tokens_of('[^a]'), msg='not singleton')
        self.assertEqual('[ab]', tokens_of('[ab]'))
        self.assertEqual('[^ab]', tokens_of('[^ab]'))
        self.assertEqual('[a-d]', tokens_of('[a-d]'))
        self.assertEqual('[^a-d]', tokens_of('[^a-d]'))
        self.assertEqual('[a-dx]', tokens_of('[a-dx]'))
        self.assertEqual('[^a-dx]', tokens_of('[^a-dx]'))
        self.assertEqual('[a-dx-z]', tokens_of('[a-dx-z]'))
        self.assertEqual('[^a-dx-z]', tokens_of('[^a-dx-z]'))

        self.assertEqual('[\\w\\d]', tokens_of('[\\w\\d]'))
        self.assertEqual('[^\\w\\d]', tokens_of('[^\\w\\d]'))

        # Not meta characters
        self.assertEqual('"-"', tokens_of('[-]'), msg='not bracket meta char')
        self.assertEqual('"]"', tokens_of('[]]'), msg='not bracket meta char')
        self.assertEqual('[^^]', tokens_of('[^^]'), msg='not bracket meta char')


if __name__ == '__main__':
    unittest.main()
