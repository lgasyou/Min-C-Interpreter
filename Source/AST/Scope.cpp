#include "Scope.h"
#include "Variable.h"
#include "AST/AST.h"

Object *Scope::lookupLocal(const String &name) {
	auto iter = variables_.find(name);
	auto end = variables_.end();
	return (iter != end) ? iter->second : nullptr;
}

Object *Scope::lookup(const String &name, bool createIfNotFound) {
	Object *value = nullptr;
	auto currentScope = this;
	while (!value && currentScope) {
		value = currentScope->lookupLocal(name);
		currentScope = currentScope->outerScope();
	}
	if (value) {
		return value;
	} 
	if (createIfNotFound) {
		return variables_[name] = new IntegerObject;
	}
	return nullptr;
}

void Scope::declarateVariable(VariableDeclaration *decl) {
	String name = decl->variableProxy()->variable()->name();
	variables_[name] = ObjectFactory(decl->variableType());
}

void Scope::declarateFunction(FunctionDeclaration *decl) {
	String name = decl->variableProxy()->variable()->name();
	auto function = variables_[name];
	if (function == nullptr) {
		variables_[name] = new FunctionObject(decl->functionBody(), decl->arguments(), decl->tokenType());
	} else {
		variables_[name]->AsFunction()->addOverloadedFunction(decl->functionBody(), decl->arguments());
	}
}

void Scope::declarateLocal(Declaration *decl) {
	switch (decl->nodeType()) {
	case AstNode::VARIABLE_DECLARATION:
		declarateVariable(static_cast<VariableDeclaration*>(decl));
		break;

	case AstNode::FUNCTION_DECLARATION:
		declarateFunction(static_cast<FunctionDeclaration*>(decl));
		break;

	default:
		UNREACHABLE();
	}
}
