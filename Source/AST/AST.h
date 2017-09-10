#pragma once

#include "Stable.h"
#include <vector>
#include "Parsing/Token.h"
#include "AstValue.h"
#include "Utils/Zone.h"

class Variable;
class VariableProxy;
class Literal;
class Expression;
class Declaration;
class Scope;

/* Basic AstNode. */
class AstNode : public ZoneObject {
public:
	enum NodeType {
		PROGRAM,

		/* Declarations */
		VARIABLE_DECLARATION,
		FUNCTION_DECLARATION,

		/* Statements */
		WHILE_STATEMENT,
		BLOCK,
		RETURN,
		EXPRESSION_STATEMENT,
		EMPTY_STATEMENT,
		IF_STATEMENT,
		BREAK_STATEMENT,
		RETURN_STATEMENT,
		OUT_STATEMENT,
		IN_STATEMENT,

		/* Expressions */
		ASSIGNMENT,
		CALL,
		UNARY_OPERATION,
		BINARY_OPERATION,
		COMPARE_OPERATION,
		VARIABLE,
		LITERAL,
	};

	int position() const { return line_; }

	NodeType nodeType() const { return node_type_; }

protected:
	AstNode(int line, NodeType type)
		: line_(line), node_type_(type) {}

private:
	int line_;
	NodeType node_type_;
};


/* Basic Statement. */
class Statement : public AstNode {
protected:
	Statement(int position, NodeType type)
		: AstNode(position, type) {}
};


class ExpressionStatement final : public Statement {
public:
	ExpressionStatement(Expression *expression, int pos)
		: Statement(pos, EXPRESSION_STATEMENT), expression_(expression) {}

	void setExpression(Expression* e) { expression_ = e; }
	Expression* expression() const { return expression_; }

private:
	Expression* expression_;
};


//class BreakableStatement : public Statement {
//protected:
//	BreakableStatement(ZoneList<const AstRawString*>* labels, int position, NodeType type)
//		: Statement(position, type) {}
//};


class Block final : public Statement {
public:
	Block(const std::vector<Statement *> &stmts, Scope *scope, int position)
		: Statement(position, BLOCK), statements_(stmts), scope_(scope) {}

	Scope *scope() const { return scope_; }
	void setScope(Scope *s) { scope_ = s; }
	void addStatement(Statement *s) { statements_.push_back(s); }

	bool isFunctionBlock() const { return is_function_block_; }
	bool isBlockEnd() const { return is_block_end_; }
	const AstValue &returnValue() const { return return_value_; }

	void setIsFunctionBlock(bool b) { is_function_block_ = b; }
	void setIsBlockEnd(bool b) { is_block_end_ = b; }
	void setReturnValue(const AstValue &v) { return_value_ = v; }

	const std::vector<Statement *> &statements() const { return statements_; }

private:
	std::vector<Statement *> statements_;
	Scope *scope_;
	AstValue return_value_ = AstValue(AstValue::VOID);
	bool is_function_block_ = false;
	bool is_block_end_ = false;
};


class WhileStatement final : public Statement {
public:
	WhileStatement(Expression *condition, Block *body, int position)
		: Statement(position, WHILE_STATEMENT), while_condition_(condition), while_body_(body) {}

	Expression *whileCondition() const { return while_condition_; }
	Block *whileBody() const { return while_body_; }

private:
	Expression *while_condition_;
	Block *while_body_;
};


class EmptyStatement final : public Statement {
public:
	EmptyStatement(int position)
		: Statement(position, EMPTY_STATEMENT) {}
};


class IfStatement : public Statement {
public:
	Expression *condition() const { return condition_; }
	Block *thenStatement() const { return then_statement_; }
	Block *elseStatement() const { return else_statement_; }

public:
	IfStatement(Expression *condition, Block *thenStatement, Block *elseStatement, int pos)
		: Statement(pos, IF_STATEMENT), condition_(condition), then_statement_(thenStatement), else_statement_(elseStatement) {}

private:
	Expression *condition_;
	Block *then_statement_;
	Block *else_statement_ = nullptr;
};


//class JumpStatement : public Statement {
//protected:
//	JumpStatement(int position, NodeType type)
//		: Statement(position, type) {}
//};


class ReturnStatement : public Statement {
public:
	ReturnStatement(Expression *returnExpr, int pos = 0)
		: Statement(pos, RETURN), return_expr_(returnExpr) {}

public:
	Expression *returnExpr() const { return return_expr_; }
private:
	Expression *return_expr_;
};


/* Output Statement. */
class OutStatement final : public Statement {
public:
	OutStatement(Literal *promptString, Expression *repeatTimes, VariableProxy *outVariableProxy, int argNum, int position)
		: Statement(position, OUT_STATEMENT), prompt_string_(promptString), repeat_times_(repeatTimes), out_variable_proxy_(outVariableProxy), arg_num_(argNum) {}

	Literal *promptString() const { return prompt_string_; }
	Expression *repeatTimes() const { return repeat_times_; }
	VariableProxy *outVariableProxy() const { return out_variable_proxy_; }
	int argNum() const { return arg_num_; }

private:
	Literal *prompt_string_ = nullptr;
	Expression *repeat_times_ = nullptr;
	VariableProxy *out_variable_proxy_ = nullptr;
	int arg_num_ = 0;
};


/* Input Statement. */
class InStatement final : public Statement {
public:
	InStatement(Literal *prompt, VariableProxy *variable, int position)
		: Statement(position, IN_STATEMENT), prompt_string_(prompt), proxy_(variable) {}

	Literal *promptString() const { return prompt_string_; }
	VariableProxy *variableProxy() const { return proxy_; }

private:
	Literal *prompt_string_;
	VariableProxy *proxy_;
};


/* Basic Declaration. */
class Declaration : public AstNode {
public:
	VariableProxy *variableProxy() const { return proxy_; }

protected:
	Declaration(VariableProxy* proxy, int pos, NodeType type)
		: AstNode(pos, type), proxy_(proxy) {}

private:
	VariableProxy *proxy_;
};

class VariableDeclaration final : public Declaration {
public:
	VariableDeclaration(VariableProxy* proxy, const Token &token, int pos)
		: Declaration(proxy, pos, VARIABLE_DECLARATION), token_type_(token.type) {}

	Token::Type variableType() const { return token_type_; }

private:
	Token::Type token_type_;
};


class FunctionDeclaration final : public Declaration {
public:
	FunctionDeclaration(VariableProxy* proxy, const Token &token,
		const std::vector<VariableDeclaration *> &arguments, Block *function, int pos)
		: Declaration(proxy, pos, FUNCTION_DECLARATION), token_type_(token.type), arguments_(arguments), body_(function) {}

	const std::vector<VariableDeclaration *> &arguments() const { return arguments_; }
	Token::Type tokenType() const { return token_type_; }
	Block *functionBody() const { return body_; }

private:
	Token::Type token_type_;
	std::vector<VariableDeclaration *> arguments_;
	Block *body_;
};


/* Basic Expression. */
class Expression : public AstNode {
protected:
	Expression(int position, NodeType type)
		: AstNode(position, type) {}
};


class Assignment final : public Expression {
public:
	Token::Type op() const { return op_; }
	VariableProxy* target() const { return target_; }
	Expression* value() const { return value_; }

	void setTarget(VariableProxy *e) { target_ = e; }
	void setValue(Expression *e) { value_ = e; }

public:
	Assignment(Token::Type op, VariableProxy *target, Expression *value, int pos)
		: Expression(pos, ASSIGNMENT), op_(op), target_(target), value_(value) {}

private:
	Token::Type op_;
	VariableProxy* target_;
	Expression* value_;
};


class Call final : public Expression {
public:
	VariableProxy *variableProxy() const { return proxy_; }
	const std::vector<Expression *> &arguments() const { return arguments_; }

public:
	Call(VariableProxy *proxy, const std::vector<Expression *> &arguments, int pos)
		: Expression(pos, CALL), proxy_(proxy), arguments_(arguments) {}

public:
	VariableProxy *proxy_;
	std::vector<Expression *> arguments_;
};


class BinaryOperation final : public Expression {
public:
	Token::Type op() const { return op_; }
	Expression* left() const { return left_; }
	void setLeft(Expression* e) { left_ = e; }
	Expression* right() const { return right_; }
	void setRight(Expression* e) { right_ = e; }

public:
	BinaryOperation(Token::Type op, Expression *left, Expression *right, int pos)
		: Expression(pos, BINARY_OPERATION), op_(op), left_(left), right_(right) {}

private:
	Token::Type op_;
	Expression* left_;
	Expression* right_;
};


class CompareOperation final : public Expression {
public:
	Token::Type op() const { return op_; }
	Expression* left() const { return left_; }
	void setLeft(Expression* e) { left_ = e; }
	Expression* right() const { return right_; }
	void setRight(Expression* e) { right_ = e; }

public:
	CompareOperation(Token::Type op, Expression* left, Expression* right, int pos)
		: Expression(pos, COMPARE_OPERATION), op_(op), left_(left), right_(right) {}

private:
	Token::Type op_;
	Expression* left_;
	Expression* right_;
};


class UnaryOperation final : public Expression {
public:
	UnaryOperation(Token::Type op, Expression *expr, int position)
		: Expression(position, UNARY_OPERATION), op_(op), expr_(expr) {}

	Token::Type op() const { return op_; }
	Expression *expression() const { return expr_; }

private:
	Token::Type op_;
	Expression *expr_;
};


class VariableProxy final : public Expression {
public:
	VariableProxy(const Token &token, int position)
		: Expression(position, VARIABLE), name_(token.value) {}

	const std::string &name() const { return name_; }
	Variable *variable();

private:
	std::string name_;
	Variable *variable_ = nullptr;
};


class Literal final : public Expression {
public:
	Literal(const Token &token, int position)
		: Expression(position, LITERAL), token_(token) {}

	AstValue *value();

private:
	Token token_;
	AstValue *value_ = nullptr;
};
