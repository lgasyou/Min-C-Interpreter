#pragma once

#include "Stable.h"
#include "AstValue.h"
#include "Utils/Zone.h"
#include "Objects.h"
#include <map>

class Variable;
class VariableDeclaration;
class FunctionDeclaration;
class Declaration;

class Scope : public ZoneObject {
public:
	typedef std::map<std::string, Object *> ObjectMap;
	typedef std::string String;

	Scope(Scope *outerScope = nullptr)
		: outer_scope_(outerScope) {}

	// Lookup a variable in this scope or outer scopes.
	// Returns the variable or variable which's just created if not found.
	Object *lookup(const String &name, bool createIfNotFound = true);

	// Declare a local variable in this scope. If the variable has been
	// declared before, this function'll do nothing.
	void declarateVariable(VariableDeclaration *decl);
	void declarateFunction(FunctionDeclaration *decl);
	void declarateLocal(Declaration *decl);

	static Scope *CopyFrom(Scope *s);

	void AddInnerScope(Scope *inner) {
		inner->sibling_ = inner_scope_;
		inner_scope_ = inner;
		inner->outer_scope_ = this;
	}

	Scope *outerScope() const { return outer_scope_; }
	void setOuterScope(Scope *os) { outer_scope_ = os; }
	Scope *innerScope() const { return inner_scope_; }
	Scope *sibling() const { return sibling_; }

private:
	// Lookup a variable in this scope. Returns the variable or nullptr if not found.
	Object *lookupLocal(const String &name);

protected:
	// Scope tree.
	Scope *outer_scope_;
	Scope *inner_scope_;
	Scope *sibling_;

	ObjectMap variables_;
};

inline Scope *Scope::CopyFrom(Scope *s) {
	auto ret = new Scope();
	ret->outer_scope_ = s->outerScope();
	ret->variables_ = s->variables_;
	return ret;
}
