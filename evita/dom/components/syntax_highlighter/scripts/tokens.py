# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class Token(object):
    def __init__(self, start, end):
        self._end = end
        self._start = start

    @property
    def is_action(self):
        return False

    @property
    def is_char(self):
        return False

    @property
    def is_label(self):
        return False

    @property
    def is_name(self):
        return False

    @property
    def is_operator(self):
        return False

    @property
    def start(self):
        return self._start

    def is_operator_of(self, char_code):
        assert char_code != 0
        return False


class ActionToken(Token):
    def __init__(self, name, start, end):
        super(ActionToken, self).__init__(start, end)
        self._name = name

    @property
    def is_action(self):
        return True

    @property
    def name(self):
        return self._name

    def __str__(self):
        return 'ActionToken(%s)' % self.name


class CharToken(Token):
    def __init__(self, char_code, start, end):
        super(CharToken, self).__init__(start, end)
        self._char_code = char_code

    @property
    def char_code(self):
        return self._char_code

    @property
    def name(self):
        if self.char_code == ord('\''):
            return "'\\''"
        if self.char_code == ord('\\'):
            return "'\\\\'"
        return "'%s'" % chr(self.char_code)

    @property
    def is_char(self):
        return True

    def __str__(self):
        return "CharToken(%s)" % self.name


class LabelToken(Token):
    def __init__(self, name, start, end):
        super(LabelToken, self).__init__(start, end)
        self._name = name

    @property
    def is_label(self):
        return True

    @property
    def name(self):
        return self._name

    def __str__(self):
        return 'LabelToken(%s)' % self.name


class NameToken(Token):
    def __init__(self, name, start, end):
        super(NameToken, self).__init__(start, end)
        self._name = name

    @property
    def is_name(self):
        return True

    @property
    def name(self):
        return self._name

    def __str__(self):
        return 'NameToken(%s)' % self.name


class OperatorToken(Token):
    def __init__(self, char_code, start, end):
        super(OperatorToken, self).__init__(start, end)
        self._char_code = char_code

    @property
    def char_code(self):
        return self._char_code

    @property
    def is_operator(self):
        return True

    def is_operator_of(self, char_code):
        return self.char_code == char_code

    def __str__(self):
        return 'OperatorToken(%s)' % chr(self.char_code)
