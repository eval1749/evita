# Copyright (c) 2016 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from pattern_ast_nodes import AstErrorNode, AstOrNode, AstPrimaryNode, \
    AstRepeatNode, AstSequenceNode
from pattern_lexer import PatternLexer


class PatternParser(object):

    def __init__(self, source):
        self._nesting = 0
        self._lexer = PatternLexer(source)

    @property
    def _is_eof(self):
        return self._lexer.is_eof

    def parse(self):
        if self._is_eof:
            return None
        self._lexer.advance()
        return self._parse_or()

    def _parse_or(self):
        members = [self._parse_sequence()]
        while not self._is_eof and self._peek_token().is_or:
            token = self._peek_token()
            self._lexer.advance()
            if self._is_eof:
                return AstErrorNode(token, 'No expression after |')
            member = self._parse_sequence()
            if member.is_or:
                for member2 in member.members:
                    members.append(member2)
            else:
                members.append(member)
        if len(members) == 1:
            return members[0]
        for member in members:
            if member.is_error:
                return member
        return AstOrNode(members)

    def _parse_primary(self):
        token = self._peek_token()
        if token.is_primary:
            self._lexer.advance()
            return AstPrimaryNode(token)
        if token.is_left_paren:
            self._lexer.advance()
            self._nesting = self._nesting + 1
            expression = self._parse_or()
            self._nesting = self._nesting - 1
            rparen = self._lexer.peek()
            if rparen.is_right_paren:
                self._lexer.advance()
                return expression
            return AstErrorNode(rparen, 'Expect right parenthesis %s' % rparen)
        if token.is_right_paren:
            if self._nesting > 0:
                return None
            self._lexer.advance()
            return AstErrorNode(token, 'Unmatcehd right parenthesis')
        self._lexer.advance()
        return AstErrorNode(token, 'Bad primary %s' % token)

    def _parse_repeat(self):
        expression = self._parse_primary()
        if self._is_eof:
            return expression
        if expression != None and expression.is_error:
            return expression
        token = self._peek_token()
        if not token.is_quantifier:
            return expression
        self._lexer.advance()
        if expression.is_repeat:
            return AstErrorNode(token, 'Can not repeat')
        if expression.token.is_boundary:
            return AstErrorNode(token,
                                'Bad repeat %s' % str(expression.token))
        return AstRepeatNode(token, expression)

    def _parse_sequence(self):
        members = [self._parse_repeat()]
        while not self._is_eof and not self._peek_token().is_or:
            member = self._parse_repeat()
            if member == None:
                break
            if member.is_sequence:
                for member2 in member.members:
                    members.append(member2)
            else:
                members.append(member)
        if len(members) == 1:
            return members[0]
        for member in members:
            if member.is_error:
                return member
        return AstSequenceNode(members)

    def _peek_token(self):
        return self._lexer.peek()
