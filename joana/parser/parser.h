// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "base/auto_reset.h"
#include "base/macros.h"
#include "joana/ast/syntax_forward.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parse.h"

namespace joana {

class SourceCode;
class SourceCodeRange;

namespace parser {

class BracketTracker;
class Lexer;

//
// Parser
//
class Parser final {
 public:
  enum class ErrorCode;
  // Expose |OperatorPrecedence| for implementing static helper functions.
  enum class OperatorPrecedence;

  Parser(ParserContext* context,
         const SourceCodeRange& range,
         const ParserOptions& options);

  ~Parser();

  // Returns root node, either |Module| or |Script|, of abstract syntax tree
  // for source code specified in |range|. The result may be partial result
  // if source code does not match grammar.
  const ast::Node& Run();

 private:
  friend class ParserTest;

  // Help class for tracking node start offset.
  class NodeRangeScope final {
   public:
    explicit NodeRangeScope(Parser* parser);
    ~NodeRangeScope();

   private:
    base::AutoReset<int> offset_holder_;

    DISALLOW_COPY_AND_ASSIGN(NodeRangeScope);
  };

  ast::NodeFactory& node_factory() const;
  const SourceCode& source_code() const;

  void AddError(const ast::Node& token, ErrorCode error_code);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);
  void AddError(ErrorCode error_code);

  void Advance();

  // Associate |jsdoc| to |node|.
  void AssociateJsDoc(const ast::Node& jsdoc, const ast::Node& node);
  bool CanPeekToken() const;
  const ast::Node& ConsumeToken();

  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);

  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);

  // Returns true if |Lexer| has a syntax of |syntax| and advance to next
  // token.
  bool ConsumeTokenIf(ast::SyntaxCode syntax);

  void ExpectPunctuator(ast::PunctuatorKind kind, ErrorCode error_code);
  void ExpectSemicolon();

  // Called when there are no more source code. Unit tests call this.
  void Finish();

  SourceCodeRange GetSourceCodeRange() const;
  const ast::Node& PeekToken() const;
  void PushBackToken(const ast::Node& token);
  void SkipCommentTokens();

  // Returns true if we stop before list element.
  bool SkipToListElement();

  // Declarations
  const ast::Node& NewEmptyName();
  const ast::Node& ParseArrowFunctionBody();
  const ast::Node& ParseClass();
  const ast::Node& ParseClassBody();
  const ast::Node& ParseClassHeritage();
  const ast::Node& ParseClassName();
  const ast::Node& ParseFunction(ast::FunctionKind kind);
  const ast::Node& ParseFunctionBody();
  const ast::Node& ParseMethod(ast::MethodKind method_kind,
                               ast::FunctionKind kind);
  const ast::Node& ParseParameterList();
  const ast::Node& ParsePropertyName();

  // Expressions
  OperatorPrecedence CategoryOf(const ast::Node& token) const;

  const ast::Node& ConvertExpressionToBindingElement(
      const ast::Node& expression,
      const ast::Node* initializer);

  std::vector<const ast::Node*> ConvertExpressionToBindingElements(
      const ast::Node& Expression);

  const ast::Node& HandleComputedMember(const ast::Node& expression);
  const ast::Node& HandleMember(const ast::Node& expression);
  const ast::Node& HandleNewExpression(const ast::Node& expression);

  OperatorPrecedence HigherPrecedenceOf(OperatorPrecedence category) const;
  const ast::Node& NewDelimiterExpression(const ast::Node& delimiter);

  // Returns a new |ast::ElisionExpression| after |node|.
  const ast::Node& NewElisionExpression(const ast::Node& node);
  const ast::Node& NewElisionExpression();
  const ast::Node& NewInvalidExpression(const SourceCodeRange& range,
                                        ErrorCode error_code);
  const ast::Node& NewInvalidExpression(const ast::Node& token,
                                        ErrorCode error_code);
  const ast::Node& NewInvalidExpression(ErrorCode error_code);

  const ast::Node& NewUnaryKeywordExpression(const ast::Node& op,
                                             const ast::Node& expression);

  const ast::Node& NewUnaryExpression(const ast::Node& op,
                                      const ast::Node& expression);

  // The entry of parsing an expression.
  const ast::Node& ParseExpression();

  // Helper function for parsing an expression enclosed by parenthesis for
  // do-while, if, switch, and while statements.
  const ast::Node& ParseParenthesisExpression();

  const ast::Node& ParseRegExp();

  const ast::Node& ParseJsDocAsExpression();
  std::vector<const ast::Node*> ParseArgumentList();
  const ast::Node& ParseArrayInitializer();
  const ast::Node& ParseAssignmentExpression();
  const ast::Node& ParseBinaryExpression(OperatorPrecedence category);
  const ast::Node& ParseCommaExpression();
  const ast::Node& ParseConditionalExpression();
  const ast::Node& ParseLeftHandSideExpression();
  const ast::Node& ParseNameAsExpression();
  const ast::Node& ParseNewExpression();
  const ast::Node& ParseObjectInitializer();
  const ast::Node& ParseParenthesis();
  const ast::Node& ParsePrimaryExpression();
  const ast::Node& ParsePropertyAfterName(const ast::Node& property_name,
                                          ast::MethodKind method_kind,
                                          ast::FunctionKind function_kind);
  const ast::Node& ParseRegExpLiteral();
  const ast::Node& ParseUnaryExpression();
  const ast::Node& ParseUpdateExpression();
  const ast::Node& ParseYieldExpression();

  // Statements
  const ast::Node& HandleLabeledStatement(const ast::Node& name);

  const ast::Node& NewInvalidStatement(ErrorCode error_code);
  const ast::Node& NewEmptyStatement(const SourceCodeRange& range);

  const ast::Node& ParseStatement();

  const ast::Node& ParseJsDocAsStatement();
  const ast::Node& ParseBreakStatement();
  const ast::Node& ParseBlockStatement();
  const ast::Node& ParseCaseClause();
  const ast::Node& ParseConstStatement();
  const ast::Node& ParseContinueStatement();
  const ast::Node& ParseDefaultLabel();
  const ast::Node& ParseDoStatement();
  const ast::Node& ParseExpressionStatement();
  const ast::Node& ParseForStatement();
  const ast::Node& ParseIfStatement();
  const ast::Node& ParseKeywordStatement();
  const ast::Node& ParseLetStatement();
  const ast::Node& ParseNameAsStatement();
  const ast::Node& ParseReturnStatement();
  const ast::Node& ParseSwitchStatement();
  const ast::Node& ParseThrowStatement();
  const ast::Node& ParseTryStatement();
  const ast::Node& ParseVarStatement();
  const ast::Node& ParseWhileStatement();
  const ast::Node& ParseWithStatement();

  // Parsing binding
  // Returns a list of |BindingElement|. This function should be called after
  // reading statement keyword, |const|, |let|, |var|, or right parenthesis
  // of function parameter list.
  std::vector<const ast::Node*> ParseBindingElements();
  const ast::Node& ParseBindingElement();

  const ast::Node& NewArrayBindingPattern(
      const std::vector<const ast::Node*>& elements,
      const ast::Node& initializer);

  const ast::Node& NewBindingCommaElement(const ast::Node& range);

  const ast::Node& NewBindingNameElement(const ast::Node& name,
                                         const ast::Node& initializer);

  const ast::Node& NewObjectBindingPattern(
      const std::vector<const ast::Node*>& elements,
      const ast::Node& initializer);

  const ast::Node& ParseArrayBindingPattern();
  const ast::Node& ParseNameBindingElement();
  const ast::Node& ParseObjectBindingPattern();

  // Map node to jsdoc
  std::unordered_map<const ast::Node*, const ast::Node*> jsdoc_map_;
  const std::unique_ptr<BracketTracker> bracket_tracker_;
  ParserContext& context_;

  // Holds @fileoverview annotation.
  const ast::Node* file_overview_ = nullptr;

  const std::unique_ptr<Lexer> lexer_;

  // True if current token and previous token is separated by at least one
  // line terminator.
  bool is_separated_by_newline_ = false;

  // The last consumed token for specify node range.
  const ast::Node* last_token_ = nullptr;
  const ParserOptions& options_;

  // Source code offset where start of node. |NodeRangeScope| manages
  // this offset.
  int node_start_ = -1;

  // List of tokens to locate comment.
  std::vector<const ast::Node*> tokens_;

  // |token_stack_| is used for look ahead, e.g. detecting whether name is
  // part of an expression or label.
  std::stack<const ast::Node*> token_stack_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_
