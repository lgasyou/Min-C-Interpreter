#include "AST.h"
#include "Variable.h"
#include <string>

AstValue *Literal::value() {
	if (!value_) {
		const auto &rawString = token_.value;
		switch (token_.type) {
		case Token::INTEGER_LITERAL:
			value_ = new AstValue(std::stoi(rawString));
			break;

		case Token::REAL_LITERAL:
			value_ = new AstValue(std::stof(rawString));
			break;

		case Token::STRING_LITERAL:
			value_ = new AstValue(rawString);
			break;

		default:
			UNREACHABLE();
		}
	}
	return value_;
}

Variable *VariableProxy::variable() {
	if (!variable_) {
		variable_ = new Variable(name());
	}
	return variable_;
}
