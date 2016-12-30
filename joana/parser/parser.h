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
#include "joana/ast/node_forward.h"
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

  // Associate |js_doc| to |node|.
  void AssociateJsDoc(const ast::JsDoc& js_doc, const ast::Node& node);
  bool CanPeekToken() const;
  const ast::Token& ConsumeToken();
  // Returns true if |Lexer| has a punctuator of |name_id| and advance to next
  // token.
  bool ConsumeTokenIf(ast::NameId keyword_id);
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool ConsumeTokenIf(ast::PunctuatorKind kind);

  template <typename Class>
  bool ConsumeTokenIf() {
    if (!CanPeekToken())
      return false;
    if (!PeekToken().Is<Class>())
      return false;
    ConsumeToken();
    return true;
  }

  void ExpectPunctuator(ast::PunctuatorKind kind, ErrorCode error_code);
  void ExpectSemicolon();

  // Called when there are no more source code. Unit tests call this.
  void Finish();

  SourceCodeRange GetSourceCodeRange() const;
  const ast::Token& PeekToken() const;
  void PushBackToken(const ast::Token& token);
  void SkipCommentTokens();

  // Returns true if we stop before list element.
  bool SkipToListElement();

  // Declarations
  const ast::Token& NewEmptyName();
  const ast::ArrowFunctionBody& ParseArrowFunctionBody();
  const ast::Class& ParseClass();
  const ast::Expression& ParseClassBody();
  const ast::Expression& ParseClassHeritage();
  const ast::Token& ParseClassName();
  const ast::Function& ParseFunction(ast::FunctionKind kind);
  const ast::Statement& ParseFunctionBody();
  const ast::Method& ParseMethod(ast::MethodKind method_kind,
                                 ast::FunctionKind kind);
  const ast::Expression& ParseParameterList();
  const ast::Expression& ParsePropertyName();

  // Expressions
  OperatorPrecedence CategoryOf(const ast::Token& token) const;

  const ast::Expression& HandleComputedMember(
      const ast::Expression& expression);
  const ast::Expression& HandleMember(const ast::Expression& expression);
  const ast::Expression& HandleNewExpression(const ast::Expression& expression);

  OperatorPrecedence HigherPrecedenceOf(OperatorPrecedence category) const;
  const ast::Expression& NewDeclarationExpression(
      const ast::Declaration& declaration);
  const ast::Expression& NewDelimiterExpression(const ast::Token& delimiter);
  const ast::Expression& NewElisionExpression();
  const ast::Expression& NewEmptyExpression();
  const ast::Expression& NewInvalidExpression(const SourceCodeRange& range,
                                              ErrorCode error_code);
  const ast::Expression& NewInvalidExpression(const ast::Token& token,
                                              ErrorCode error_code);
  const ast::Expression& NewInvalidExpression(ErrorCode error_code);
  const ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  const ast::Expression& NewUnaryExpression(const ast::Token& op,
                                            const ast::Expression& expression);

  // The entry of parsing an expression.
  const ast::Expression& ParseExpression();

  // Helper function for parsing an expression enclosed by parenthesis for
  // do-while, if, switch, and while statements.
  const ast::Expression& ParseParenthesisExpression();

  const ast::RegExp& ParseRegExp();

  const ast::Expression& ParseJsDocAsExpression();
  std::vector<const ast::Expression*> ParseArgumentList();
  const ast::Expression& ParseArrayLiteralExpression();
  const ast::Expression& ParseAssignmentExpression();
  const ast::Expression& ParseBinaryExpression(OperatorPrecedence category);
  const ast::Expression& ParseCommaExpression();
  const ast::Expression& ParseConditionalExpression();
  const ast::Expression& ParseFunctionExpression(ast::FunctionKind kind);
  const ast::Expression& ParseLeftHandSideExpression();
  const ast::Expression& ParseMethodExpression(ast::MethodKind method_kind,
                                               ast::FunctionKind kind);
  const ast::Expression& ParseNameAsExpression();
  const ast::Expression& ParseNewExpression();
  const ast::Expression& ParseObjectLiteralExpression();
  const ast::Expression& ParseParenthesis();
  const ast::Expression& ParsePrimaryExpression();
  const ast::Expression& ParsePropertyAfterName(
      const ast::Expression& property_name,
      ast::MethodKind method_kind,
      ast::FunctionKind function_kind);
  const ast::Expression& ParseRegExpLiteral();
  const ast::Expression& ParseUnaryExpression();
  const ast::Expression& ParseUpdateExpression();
  const ast::Expression& ParseYieldExpression();

  // Statements
  const ast::Statement& HandleLabeledStatement(const ast::Name& name);

  const ast::Statement& NewInvalidStatement(ErrorCode error_code);
  const ast::Statement& NewEmptyStatement(const SourceCodeRange& range);

  const ast::Statement& ParseStatement();

  const ast::Statement& ParseJsDocAsStatement();
  const ast::Statement& ParseBreakStatement();
  const ast::Statement& ParseBlockStatement();
  const ast::Statement& ParseCaseClause();
  const ast::Statement& ParseClassStatement();
  const ast::Statement& ParseConstStatement();
  const ast::Statement& ParseContinueStatement();
  const ast::Statement& ParseDefaultLabel();
  const ast::Statement& ParseDoStatement();
  const ast::Statement& ParseExpressionStatement();
  const ast::Statement& ParseForStatement();
  const ast::Statement& ParseFunctionStatement(ast::FunctionKind kind);
  const ast::Statement& ParseIfStatement();
  const ast::Statement& ParseKeywordStatement();
  const ast::Statement& ParseLetStatement();
  const ast::Statement& ParseNameAsStatement();
  const ast::Statement& ParseReturnStatement();
  const ast::Statement& ParseSwitchStatement();
  const ast::Statement& ParseThrowStatement();
  const ast::Statement& ParseTryStatement();
  const ast::Statement& ParseVarStatement();
  const ast::Statement& ParseWhileStatement();
  const ast::Statement& ParseWithStatement();

  // Parsing binding
  // Returns a list of |BindingElement|. This function should be called after
  // reading statement keyword, |const|, |let|, |var|, or right parenthesis
  // of function parameter list.
  std::vector<const ast::BindingElement*> ParseBindingElements();
  const ast::BindingElement& ParseBindingElement();

  const ast::BindingElement& NewArrayBindingPattern(
      const std::vector<const ast::BindingElement*>& elements,
      const ast::Expression& initializer);

  const ast::BindingElement& NewBindingCommaElement(const ast::Token& range);

  const ast::BindingElement& NewBindingNameElement(
      const ast::Name& name,
      const ast::Expression& initializer);

  const ast::BindingElement& NewObjectBindingPattern(
      const std::vector<const ast::BindingElement*>& elements,
      const ast::Expression& initializer);

  const ast::BindingElement& ParseArrayBindingPattern();
  const ast::BindingElement& ParseNameBindingElement();
  const ast::BindingElement& ParseObjectBindingPattern();

  // Map node to js_doc
  std::unordered_map<const ast::Node*, const ast::JsDoc*> js_doc_map_;
  const std::unique_ptr<BracketTracker> bracket_tracker_;
  ParserContext& context_;

  // Holds @fileoverview annotation.
  const ast::JsDocDocument* file_overview_ = nullptr;

  const std::unique_ptr<Lexer> lexer_;

  // True if current token and previous token is separated by at least one
  // line terminator.
  bool is_separated_by_newline_ = false;

  // The last consumed token for specify node range.
  const ast::Token* last_token_ = nullptr;
  const ParserOptions& options_;

  // Source code offset where start of node. |NodeRangeScope| manages
  // this offset.
  int node_start_ = -1;

  // List of tokens to locate comment.
  std::vector<const ast::Token*> tokens_;

  // |token_stack_| is used for look ahead, e.g. detecting whether name is
  // part of an expression or label.
  std::stack<const ast::Token*> token_stack_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace parser
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_
