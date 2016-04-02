# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class AstNode(object):
    def __init__(self, token):
        self._token = token

    @property
    def is_error(self):
        return False

    @property
    def is_lazy(self):
        return False

    @property
    def is_or(self):
        return False

    @property
    def is_primary(self):
        return False

    @property
    def is_repeat(self):
        return False

    @property
    def is_sequence(self):
        return False

    @property
    def token(self):
        return self._token


class AstErrorNode(AstNode):
    def __init__(self, token, message):
        super(AstErrorNode, self).__init__(token)
        self._message = message

    @property
    def is_error(self):
        return True

    def __str__(self):
        return 'error("%s")' % self._message.replace('"', '\\"')


class AstOrNode(AstNode):
    def __init__(self, members):
        super(AstOrNode, self).__init__(members[0].token)
        assert(len(members) >= 2)
        self._members = members

    @property
    def is_or(self):
        return True

    @property
    def members(self):
        return self._members

    def __str__(self):
        return 'or(%s)' % ', '.join([str(member) for member in self._members])


class AstPrimaryNode(AstNode):
    def __init__(self, token):
        super(AstPrimaryNode, self).__init__(token)

    @property
    def is_primary(self):
        return True

    def __str__(self):
        return str(self.token)


class AstRepeatNode(AstNode):
    def __init__(self, token, expression):
        super(AstRepeatNode, self).__init__(token)
        self._expression = expression

    @property
    def expression(self):
        return self._expression

    @property
    def is_infinity(self):
        return self.token.is_infinity

    @property
    def is_lazy(self):
        return self.token.is_lazy

    @property
    def is_repeat(self):
        return True

    @property
    def max_count(self):
        return self.token.max_count

    @property
    def min_count(self):
        return self.token.min_count

    def __str__(self):
        return 'repeat(%s, %s)' % (self.token, self._expression)


class AstSequenceNode(AstNode):
    def __init__(self, members):
        super(AstSequenceNode, self).__init__(members[0].token)
        assert(len(members) >= 2)
        self._members = members

    @property
    def is_sequence(self):
        return True

    @property
    def members(self):
        return self._members

    def __str__(self):
        return 'sequence(%s)' % ', '.join([str(member)
                                           for member in self._members])
