# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import unittest

from pattern_parser import PatternParser

def parse(source):
    parser = PatternParser(source)
    return str(parser.parse())

class PatternParserTest(unittest.TestCase):
    def test_complex(self):
        # Double quoted string with "/" as escape character
        self.assertEqual('sequence("\\"", '
                         'repeat(*, or([^/\\"], sequence("/", .))), "\\"")',
                         parse('"([^/"]|/.)*"'))
        # C-comment replace "@" as "[*]" for real usage
        self.assertEqual('sequence("/", "@", repeat(*, [^*]), '
                         'repeat(+, "@"), [^/*], repeat(*, [^*]), '
                         'repeat(+, "@"), "@", "/")',
                         parse('/@[^*]*@+([^/*][^*]*@+)@/'))

    def test_error(self):
        self.assertEqual('error("Bad primary *")', parse('*'))
        self.assertEqual('error("Bad primary *")', parse('**'))
        self.assertEqual('error("Bad primary +")', parse('a*+'))
        self.assertEqual('error("Can not repeat")', parse('(a*)+'))

        self.assertEqual('error("Bad primary *")', parse('*'))
        self.assertEqual('error("Bad primary +")', parse('+'))
        self.assertEqual('error("Bad primary ?")', parse('?'))

        self.assertEqual('error("Bad repeat \\b")', parse('\\b?'))
        self.assertEqual('error("Bad repeat \\b")', parse('\\b*c'))
        self.assertEqual('error("Bad repeat ^")', parse('^?'))

    def test_group(self):
        self.assertEqual('or("a", "b")', parse('(a|b)'))
        self.assertEqual('or("a", "b", "c")', parse('(a|(b|c))'))
        self.assertEqual('repeat(+, or("a", "b"))', parse('(a|b)+'))

    def test_lazy_quantifiers(self):
        self.assertEqual('sequence("/", "*", repeat(*?, .), "*", "/")',
                         parse('/[*].*?[*]/'))

    def test_or(self):
        self.assertEqual('or(sequence("a", "b"), sequence("c", "d"))',
                         parse('ab|cd'))
        self.assertEqual('or(repeat(+, "a"), repeat(+, "c"))',
                         parse('a+|c+'))
        self.assertEqual('or("a", "b", "c")', parse('a|b|c'))
        self.assertEqual('error("Bad primary |")', parse('|'))

    def test_primary(self):
        self.assertEqual('"a"', parse('a'))
        self.assertEqual('.', parse('.'))
        self.assertEqual('\\w', parse('\\w'))
        self.assertEqual('[a-z]', parse('[a-z]'))

    def test_repeat(self):
        self.assertEqual('repeat(*, "a")', parse('a*'))
        self.assertEqual('repeat(*, [a-z])', parse('[a-z]*'))
        self.assertEqual('repeat(+, "a")', parse('a+'))
        self.assertEqual('repeat(+, [a-z])', parse('[a-z]+'))
        self.assertEqual('repeat(?, "a")', parse('a?'))
        self.assertEqual('repeat(?, [a-z])', parse('[a-z]?'))

    def test_sequence(self):
        self.assertEqual('sequence("a", "b", "c")', parse('abc'))
        self.assertEqual('sequence("a", repeat(*, "b"))', parse('ab*'))
        self.assertEqual('sequence("a", repeat(*, "b"), "c")', parse('ab*c'))
        self.assertEqual('sequence("a", repeat(+, "b"))', parse('ab+'))
        self.assertEqual('sequence("a", repeat(+, "b"), "c")', parse('ab+c'))
        self.assertEqual('sequence("a", repeat(?, "b"))', parse('ab?'))
        self.assertEqual('sequence("a", repeat(?, "b"), "c")', parse('ab?c'))


if __name__ == '__main__':
    unittest.main()
