#pragma once

#include <string>
#include <stack>
#include "AstValue.h"
#include "AST.h"

class AstNode;
class OutStatement;
class InStatement;
class VariableProxy;
class Variable;

/* Basic Visitor. */
class AstNodeVisitor {
public:
	virtual ~AstNodeVisitor() {}

	// Visits root and all its children.
	virtual void visit(AstNode *root) = 0;
};


class Analyzer : public AstNodeVisitor {
public:
	void visit(AstNode *root) override;

private:
	/* Statements */
	void visitStatement(Statement *node);
	void visitBlock(Block *node);
	void visitInStatement(InStatement *node);
	void visitOutStatement(OutStatement *node);
	void visitWhileStatement(WhileStatement *node);
	void visitIfStatement(IfStatement *node);
	AstValue visitRuturnStatement(ReturnStatement *node);
	void restoreStack();

	/* Expression Statements */
	AstValue visitExpressionStatement(ExpressionStatement *node);
	AstValue visitAssignment(Assignment *node);
	AstValue visitCall(Call *node);
	std::vector<AstValue> getCallArgValues(const std::vector<Expression *> &argDecls);

	/* Operations */
	AstValue visitUnaryOperation(UnaryOperation *node);
	AstValue visitCompareOperation(CompareOperation *node);
	AstValue visitBinaryOperation(BinaryOperation *node);

	/* Low-level visits */
	AstValue visitExpression(Expression *node);
	Variable visitVariableProxy(VariableProxy *node);
	AstValue visitLiteral(Literal *literal);

private:
	Scope *current_scope_ = nullptr;
	std::stack<Block *> block_stack_;
	std::stack<Scope *> scope_stack_;
};
