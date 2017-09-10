#pragma once

#include "Stable.h"
#include "Parsing/Token.h"
#include "Utils/Zone.h"

class VariableDeclaration;
class AstValue;
class IntegerObject;
class RealObject;
class StringObject;
class FunctionObject;
class Block;


class Object : public ZoneObject {
public:
	enum Type {
		INTEGER = Token::INT,
		REAL = Token::REAL,
		STRING = Token::STRING,
		FUNCTION,
	};

	bool isInteger() const { return type_ == INTEGER; }
	IntegerObject *AsInteger();

	bool isReal() const { return type_ == REAL; }
	RealObject *AsReal();

	bool isString() const { return type_ == STRING; }
	StringObject *AsString();

	bool isFunction() const { return type_ == FUNCTION; }
	FunctionObject *AsFunction();

	Type type() const { return type_; }

	AstValue operator=(const AstValue &rhs);
	AstValue toAstValue();
	static Object *FromAstValue(const AstValue &rhs);

protected:
	Object(Type t)
		: type_(t) {}

private:
	Type type_;
};


class IntegerObject : public Object {
public:
	IntegerObject(int val = 0)
		: Object(INTEGER), val_(val) {}

	void setValue(int val) { val_ = val; }
	int value() { return val_; }

private:
	int val_;
};


class RealObject : public Object {
public:
	RealObject(float val = 0.0f)
		: Object(REAL), val_(val) {}

	void setValue(float val) { val_ = val; }
	float value() { return val_; }

private:
	float val_;
};


class StringObject : public Object {
public:
	StringObject(const std::string &val = std::string())
		: Object(STRING), val_(val) {}

	void setValue(const std::string &val) { val_ = val; }
	const std::string &value() { return val_; }

private:
	std::string val_;
};


class FunctionObject : public Object {
public:
	struct OverloadedFunction {
		std::vector<VariableDeclaration *> actual_arguments;
		Block *block;
	};

	FunctionObject(Block *block, const std::vector<VariableDeclaration *> &actualArguments, Token::Type type)
		: Object(FUNCTION), return_type_(type) {
		addOverloadedFunction(block, actualArguments);
	}

	Block * setup(const std::vector<AstValue> &formalArguments);
	void addOverloadedFunction(Block *block, const std::vector<VariableDeclaration *> &actualArguments);

private:
	OverloadedFunction &getMatchedFunction(const std::vector<AstValue> &formalArguments);

private:
	std::vector<OverloadedFunction> overloaded_functions_;
	Token::Type return_type_;
};

inline Object *ObjectFactory(Object::Type t) {
	switch (t) {
	case Object::INTEGER:
		return new IntegerObject;

	case Object::REAL:
		return new RealObject;

	case Object::STRING:
		return new StringObject;

	case Object::FUNCTION:
	default:
		UNREACHABLE();
	}
}

inline Object *ObjectFactory(Token::Type t) {
	Object::Type objType = static_cast<Object::Type>(t);
	return ObjectFactory(objType);
}
