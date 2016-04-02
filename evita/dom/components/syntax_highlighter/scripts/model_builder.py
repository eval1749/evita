# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from model import Action, Document, Rule, State

class Builder(object):
    def __init__(self):
        pass

    def error(self, token, message):
        raise Exception('%s: %s' % (token.start, message))


class ActionBuilder(Builder):
    def __init__(self, token):
        super(ActionBuilder, self).__init__()
        if not token.is_action:
            self.error(token, 'Expect action')
        self._arguments = []
        self._token = token

    def add_argument(self, argument):
        self._arguments.append(argument)

    def build(self):
        return Action(self._token, self._arguments)


class RuleBuilder(Builder):
    def __init__(self, token):
        super(RuleBuilder, self).__init__()
        if not (token.is_name or token.is_char):
            self.error(token, 'Expect rule condition %s' % token)
        self._actions = []
        self._token = token

    def add_action(self, action):
        self._actions.append(action)

    def build(self):
        return Rule(self._token, self._actions)


class StateBuilder(Builder):
    def __init__(self, token):
        super(StateBuilder, self).__init__()
        self._rules = []
        self._token = token

    def add_rule(self, rule):
        self._rules.append(rule)

    def build(self):
        return State(self._token, self._rules)


class DocumentBuilder(Builder):
    def __init__(self):
        super(DocumentBuilder, self).__init__()
        self._builders = []
        self._states = []

    def add_argument(self, argument):
        action_builder = self._builders[-1]
        assert isinstance(action_builder, ActionBuilder)
        action_builder.add_argument(argument)

    def build(self):
        assert len(self._builders) == 0, self._builders
        assert len(self._states) > 0, self._states
        return Document(self._states)

    def end_action(self):
        action_builder = self._builders.pop()
        assert isinstance(action_builder, ActionBuilder)
        rule_builder = self._builders[-1]
        assert isinstance(rule_builder, RuleBuilder)
        rule_builder.add_action(action_builder.build())

    def end_rule(self):
        rule_builder = self._builders.pop()
        assert isinstance(rule_builder, RuleBuilder)
        state_builder = self._builders[-1]
        assert isinstance(state_builder, StateBuilder)
        state_builder.add_rule(rule_builder.build())

    def end_state(self):
        state_builder = self._builders.pop()
        assert isinstance(state_builder, StateBuilder)
        self._states.append(state_builder.build())

    def start_action(self, token):
        self._builders.append(ActionBuilder(token))

    def start_rule(self, token):
        self._builders.append(RuleBuilder(token))

    def start_state(self, token):
        self._builders.append(StateBuilder(token))
