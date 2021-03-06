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
	initContext(node);
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
	restoreContext();
}


void Analyzer::visitStatement(Statement *node) {
	switch (node->nodeType()) {
	case AstNode::VARIABLE_DECLARATION:
		VISIT(VariableDeclaration, node);
		break;

	case AstNode::FUNCTION_DECLARATION:
		VISIT(FunctionDeclaration, node);
		break;

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

	case AstNode::FOR_STATEMENT:
		VISIT(ForStatement, node);
		break;

	case AstNode::IF_STATEMENT:
		VISIT(IfStatement, node);
		break;

	case AstNode::BLOCK:
		VISIT(Block, node);
		break;

	case AstNode::DO_UNTIL_STATEMENT:
		VISIT(DoUntilStatement, node);
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
	switch ((int)(node->outMembers().size())) {

	case 1:
	{
		AstValue outValue = visitExpression(node->outMembers()[0]);
		std::cout << outValue;
		break;
	}

	case 2:
	{
		AstValue firstValue = visitExpression(node->outMembers()[0]);
		AstValue secondValue = visitExpression(node->outMembers()[1]);
		if (firstValue.type() == AstValue::INTEGER) {
			for (int i = 0; i != firstValue.toInt(); i++) {
				std::cout << secondValue;
			}
		} else {
			std::cout << firstValue;
			std::cout << secondValue;
		}
		break;
	}

	case 3:
	{
		AstValue firstValue = visitExpression(node->outMembers()[0]);
		AstValue secondValue = visitExpression(node->outMembers()[1]);
		AstValue thirdValue = visitExpression(node->outMembers()[2]);
		for (int i = 0; i != firstValue.toInt(); i++) {
			std::cout << secondValue;
		}
		std::cout << thirdValue;
		break;
	}

	default:
		UNREACHABLE();
		break;
	}
}

void Analyzer::visitWhileStatement(WhileStatement *node) {
	while (visitExpression(node->whileCondition())) {
		visit(node->body());
	}
}

void Analyzer::visitForStatement(ForStatement *node) {
	auto init = node->init();
	auto cond = node->cond();
	auto body = node->body();

	// Visit init and next Statement in the scope of "for body".
	initContext(body);
	visitStatement(init);
	body->setScope(current_scope_);
	while (!body->isBlockEnd() && (!cond || visitExpression(cond))) {
		restoreContext();
		visitBlock(body);
		if (!body->isBlockEnd()) {
			initContext(body);
		}
	}
}

void Analyzer::visitDoUntilStatement(DoUntilStatement *node) {
	do {
		visit(node->doBody());
	} while (!visitExpression(node->untilCondition()));
}

void Analyzer::visitIfStatement(IfStatement *node) {
	if (visitExpression(node->condition())) {
		//		DBG_PRINT << "If n <= 1:\n";
		visitBlock(node->thenStatement());
	} else if (node->elseStatement()) {
		//		DBG_PRINT << "Else:\n";
		visitBlock(node->elseStatement());
	} else if (node->elseIfStatement()) {
		VISIT(IfStatement, node->elseIfStatement());
	}
}

AstValue Analyzer::visitRuturnStatement(ReturnStatement *node) {
	if (node->returnExpr()) {
		return visitExpression(node->returnExpr());
	}
	return AstValue(AstValue::VOID);
}

void Analyzer::initContext(Block *block) {
	block->setIsBlockEnd(false);
	block->setReturnValue(AstValue(AstValue::VOID));

	block_stack_.push(block);

	Scope *s = Scope::CopyFrom(block->scope());
	s->setOuterScope(current_scope_);
	current_scope_ = s;
	scope_stack_.push(current_scope_);
}

void Analyzer::restoreContext() {
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

void Analyzer::visitFunctionDeclaration(FunctionDeclaration *decl) {
	current_scope_->declarateFunction(decl);
}

void Analyzer::visitVariableDeclaration(VariableDeclaration *decl) {
	current_scope_->declarateVariable(decl);
	auto name = decl->variableProxy()->variable()->name();
	auto var = current_scope_->lookup(name);
	if (decl->initializer()) {
		auto initializerValue = visitExpression(decl->initializer());
		*var = initializerValue;
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
		if (node->position() == 0)throw FuncDecException("main");
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

	case Token::INV:
		//DBG_PRINT << (visitExpression(left) ^ visitExpression(right));
		return (visitExpression(left) ^ visitExpression(right));

	default:
		throw OpException(node->position());
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
		throw OpException(node->position());
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
		throw OpException(node->position());
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
		throw OpException(node->position());
	}
}

Variable Analyzer::visitVariableProxy(VariableProxy *node) {
	return *node->variable();
}

AstValue Analyzer::visitLiteral(Literal *literal) {
	return *literal->value();
}