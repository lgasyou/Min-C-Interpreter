#include "Analyzer.h"
#include "AST.h"
#include "Variable.h"
#include "Objects.h"
#include "Scope.h"
#include "Utils\Exceptions.h"
#include <map>

// A macro used to cast NODE to NODE_TYPE* 
// and call the function which is suitable for the NODE.
// Use like:
//	VISIT(OutStatement, node);
#define VISIT(NODE_TYPE, NODE) visit##NODE_TYPE(static_cast<NODE_TYPE *>(NODE))

void Analyzer::visit(AstNode *root) {
	VISIT(Statement, root);
}

void Analyzer::visitBlock(Block *node) {
	node->setIsBlockEnd(false);
	node->setReturnValue(AstValue(AstValue::VOID));

	block_stack_.push(node);

	auto cs = Scope::CopyFrom(node->scope());
	cs->setOuterScope(current_scope_);
	current_scope_ = cs;
	scope_stack_.push(current_scope_);

	for (auto s : node->statements()) {
		if (node->isBlockEnd()) {
			break;
		}

		if (s->nodeType() == AstNode::RETURN) {
			auto rval = visitRuturnStatement((ReturnStatement *)s);
			node->setReturnValue(rval);
			node->setIsBlockEnd(true);
			break;
		}
		visitStatement(s);
	}
	restoreStack();
}


void Analyzer::visitStatement(Statement *node) {
	switch (node->nodeType()) {
	case AstNode::OUT_STATEMENT:
		VISIT(OutStatement, node);
		break;

	case AstNode::IN_STATEMENT:
		VISIT(InStatement, node);
		break;

	case AstNode::EXPRESSION_STATEMENT:
		VISIT(ExpressionStatement, node);
		break;

	case AstNode::EMPTY_STATEMENT:
		// Do nothing and jump empty statment.
		break;

	case AstNode::WHILE_STATEMENT:
		VISIT(WhileStatement, node);
		break;

	case AstNode::IF_STATEMENT:
		VISIT(IfStatement, node);
		break;

	case AstNode::BLOCK:
		VISIT(Block, node);
		break;

	default:
		UNREACHABLE();
	}
}

void Analyzer::visitInStatement(InStatement *node) {
	if (node->promptString()) {
		std::cout << visitLiteral(node->promptString());
	}

	auto proxy = node->variableProxy();
	auto &name = proxy->variable()->name();
	auto var = current_scope_->lookup(name);
	AstValue input{ static_cast<AstValue::Type>(var->type()) };
	std::cin >> input;
	*var = input;
}

void Analyzer::visitOutStatement(OutStatement *node) {
	int times = 1;

	if (node->repeatTimes() != nullptr) {
		switch ((node->repeatTimes())->nodeType()) {
		case AstNode::VARIABLE: {
			const auto &name = static_cast<VariableProxy *>(node->repeatTimes())->variable()->name();
			const auto &var = current_scope_->lookup(name);
			times = var->AsInteger()->value();
			break;
		}

		case AstNode::LITERAL:
			times = static_cast<Literal *>(node->repeatTimes())->value()->toInt();
			break;

		default:
			UNREACHABLE();
		}
	}

	AstValue VariableValue;
	if (node->outVariableProxy() != nullptr) {
		const auto &name = node->outVariableProxy()->variable()->name();
		const auto &var = current_scope_->lookup(name);
		VariableValue = var->toAstValue();
	}

	// ------------------------------
	switch (node->argNum()) {
	case 1:
		if (node->promptString() != nullptr) {
			std::cout << visitLiteral(node->promptString());
		} else {
			std::cout << VariableValue;
		}
		break;
	case 2:
		if (node->repeatTimes() != nullptr) {
			if (node->promptString() != nullptr) {
				for (int i = 0; i != times; ++i) {
					std::cout << visitLiteral(node->promptString());
				}
			} else {
				for (int i = 0; i != times; ++i) {
					std::cout << VariableValue;
				}
			}
		} else {
			std::cout << visitLiteral(node->promptString());
			std::cout << VariableValue;
		}
		break;

	case 3:
		for (int i = 0; i != times; ++i) {
			std::cout << visitLiteral(node->promptString());
		}
		std::cout << VariableValue;
		break;

	default:
		UNREACHABLE();
	}
}

void Analyzer::visitWhileStatement(WhileStatement *node) {
	while (visitExpression(node->whileCondition())) {
		visit(node->whileBody());
	}
}

void Analyzer::visitIfStatement(IfStatement *node) {
	if (visitExpression(node->condition())) {
		visitBlock(node->thenStatement());
	} else if (node->elseStatement()) {
		visitBlock(node->elseStatement());
	}
}

AstValue Analyzer::visitRuturnStatement(ReturnStatement *node) {
	if (node->returnExpr()) {
		return visitExpression(node->returnExpr());
	}
	return AstValue(AstValue::VOID);
}

void Analyzer::restoreStack() {
	Block *poppedBlock = block_stack_.top();
	block_stack_.pop();
	scope_stack_.pop();
	if (!block_stack_.empty()) {
		auto b = block_stack_.top();
		if (!poppedBlock->isFunctionBlock()) {
			b->setIsBlockEnd(poppedBlock->isBlockEnd());
		}
		b->setReturnValue(poppedBlock->returnValue());
		current_scope_ = scope_stack_.top();
	}
}

AstValue Analyzer::visitExpressionStatement(ExpressionStatement *node) {
	auto expression = node->expression();
	switch (expression->nodeType()) {
	case AstNode::ASSIGNMENT:
		return VISIT(Assignment, expression);

	case AstNode::CALL:
		return VISIT(Call, expression);

	default:
		UNREACHABLE();
	}
}

AstValue Analyzer::visitAssignment(Assignment *node) {
	std::string targetName = node->target()->variable()->name();
	auto var = current_scope_->lookup(targetName);
	if ((node->value())->nodeType() == AstNode::ASSIGNMENT) {
		return *var = VISIT(Assignment, node->value());
	}
	return *var = visitExpression(node->value());
}

AstValue Analyzer::visitCall(Call *node) {
	auto funName = node->variableProxy()->variable()->name();
	auto argValues = getCallArgValues(node->arguments());
	auto function = current_scope_->lookup(funName);
	if (function->type() != Object::FUNCTION) {
		throw FuncDecException(node->position());
	}
	auto readyBlock = function->AsFunction()->setup(argValues);
	visitBlock(readyBlock);
	return readyBlock->returnValue();
}

std::vector<AstValue> Analyzer::getCallArgValues(const std::vector<Expression *> &argDecls) {
	std::vector<AstValue> ret;
	for (auto e : argDecls) {
		ret.push_back(visitExpression(e));
	}
	return ret;
}

AstValue Analyzer::visitBinaryOperation(BinaryOperation *node) {
	auto left = node->left();
	auto right = node->right();
	switch (node->op()) {
	case Token::ADD:
	case Token::STRING_CONCAT:
		return visitExpression(left) + visitExpression(right);

	case Token::SUB:
	case Token::STRING_DELETE:
		return visitExpression(left) - visitExpression(right);

	case Token::MUL:
		return visitExpression(left) * visitExpression(right);

	case Token::MOD:
		return visitExpression(left) % visitExpression(right);

	case Token::DIV:
		return visitExpression(left) / visitExpression(right);

	default:
		UNREACHABLE();
	}
}

AstValue Analyzer::visitUnaryOperation(UnaryOperation *node) {
	switch (node->op()) {
	case Token::ADD:
		return visitExpression(node->expression());

	case Token::SUB:
		return AstValue(-1) * visitExpression(node->expression());

	case Token::NOT:
		return !visitExpression(node->expression());

	default:
		UNREACHABLE();
	}
}

AstValue Analyzer::visitCompareOperation(CompareOperation *node) {
	auto left = node->left();
	auto right = node->right();
	switch (node->op()) {
	case Token::LT:
		return visitExpression(left) < visitExpression(right);

	case Token::GT:
		return visitExpression(left) > visitExpression(right);

	case Token::LTE:
		return visitExpression(left) <= visitExpression(right);

	case Token::GTE:
		return visitExpression(left) >= visitExpression(right);

	case Token::EQ:
		return visitExpression(left) == visitExpression(right);

	case Token::NE:
		return visitExpression(left) != visitExpression(right);

	case Token::AND:
		return visitExpression(left) && visitExpression(right);

	case Token::OR:
		return visitExpression(left) || visitExpression(right);

	default:
		UNREACHABLE();
	}
}

AstValue Analyzer::visitExpression(Expression *node) {
	auto type = node->nodeType();
	switch (type) {
	case AstNode::ASSIGNMENT:
		return VISIT(Assignment, node);

	case AstNode::UNARY_OPERATION:
		return VISIT(UnaryOperation, node);

	case AstNode::BINARY_OPERATION:
		return VISIT(BinaryOperation, node);

	case AstNode::COMPARE_OPERATION:
		return VISIT(CompareOperation, node);

	case AstNode::VARIABLE: {
		const std::string &name = static_cast<VariableProxy*>(node)->variable()->name();
		return current_scope_->lookup(name)->toAstValue();
	}

	case AstNode::LITERAL:
		return VISIT(Literal, node);

	case AstNode::CALL:
		return VISIT(Call, node);

	default:
		UNREACHABLE();
	}
}

Variable Analyzer::visitVariableProxy(VariableProxy *node) {
	return *node->variable();
}

AstValue Analyzer::visitLiteral(Literal *literal) {
	return *literal->value();
}