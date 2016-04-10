# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class Node(object):

    def __init__(self, token):
        self._token = token

    @property
    def token(self):
        return self._token


class Action(Node):

    def __init__(self, token, arguments):
        super(Action, self).__init__(token)
        self._arguments = arguments

    @property
    def arguments(self):
        return self._arguments

    def __str__(self):
        arguments = [argument.name for argument in self.arguments]
        if len(arguments) == 0:
            return 'Action(%s)' % self.token.name
        return 'Action(%s, %s)' % (self.token.name, ', '.join(arguments))


class Document(Node):

    def __init__(self, states):
        super(Document, self).__init__(None)
        self._states = states

    @property
    def states(self):
        return self._states


class Rule(Node):

    def __init__(self, token, actions):
        super(Rule, self).__init__(token)
        self._actions = actions

    @property
    def actions(self):
        return self._actions

    def __str__(self):
        return 'Rule(%s)' % self.token.name


class State(Node):

    def __init__(self, token, rules):
        super(State, self).__init__(token)
        self._rules = rules

    @property
    def rules(self):
        return self._rules

    def __str__(self):
        return 'State(%s)' % self.token.name
