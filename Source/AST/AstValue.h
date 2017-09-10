#pragma once

#include "Stable.h"
#include <string>
#include <iostream>
#include <cmath>
#include "Objects.h"
#include "Parsing/Token.h"
#include "Utils/Zone.h"

/* AstValue is either a string, a integer or a real number. */
class AstValue : public ZoneObject {
	friend std::istream &operator>>(std::istream &is, AstValue &var);
public:
	enum Type {
		INTEGER = Token::INT,
		REAL	= Token::REAL,
		STRING	= Token::STRING,
		VOID,
	};

	explicit AstValue(int integer = 0);
	explicit AstValue(float real);
	explicit AstValue(const std::string &string);
	explicit AstValue(const char *str);
	explicit AstValue(bool boolean);
	explicit AstValue(Type type);
	explicit AstValue(Token::Type type);
	AstValue(const AstValue &rhs);

	~AstValue();

	int toInt() const;
	float toReal() const;
	const std::string &toString() const;
	bool isVoid() const { return type_ == VOID; }

	Type type() const { return type_; }

	AstValue &operator=(const AstValue &rhs);
	AstValue operator+(const AstValue &rhs);
	AstValue operator-(const AstValue &rhs);
	AstValue operator*(const AstValue &rhs);
	AstValue operator/(const AstValue &rhs);
	AstValue operator%(const AstValue &rhs);
	AstValue operator^(const AstValue &rhs);

	AstValue operator==(const AstValue &rhs);
	AstValue operator!=(const AstValue &rhs);
	AstValue operator<(const AstValue &rhs);
	AstValue operator<=(const AstValue &rhs);
	AstValue operator>(const AstValue &rhs);
	AstValue operator>=(const AstValue &rhs);

	AstValue operator&&(const AstValue &rhs);
	AstValue operator||(const AstValue &rhs);
	AstValue operator!();

	operator bool();

private:
	union {
		int integer;
		float real;
		std::string *string;
	} var;

	Type type_;
};

inline int AstValue::toInt() const {
	return var.integer;
}

inline float AstValue::toReal() const {
	return type() == REAL ? var.real : var.integer;
}

inline const std::string &AstValue::toString() const {
	return *var.string;
}

inline std::istream &operator>>(std::istream &is, AstValue &var) {
	switch (var.type()) {
	case AstValue::INTEGER: {
		int tmp;
		is >> tmp;
		var.var.integer = tmp;
		break;
	}

	case AstValue::REAL: {
		float tmp;
		is >> tmp;
		var.var.real = tmp;
		break;
	}

	case AstValue::STRING:{
		delete var.var.string;
		std::string tmp;
		if (std::cin.peek() == '\n') {
			getchar();
		}
		std::getline(is, tmp);
		var.var.string = new std::string(tmp);
		break;
	}
	}
	return is;
}

inline std::ostream &operator<<(std::ostream &os, const AstValue &var) {
	switch (var.type()) {
	case AstValue::INTEGER:
		os << var.toInt();
		break;

	case AstValue::REAL:
		os << var.toReal();
		break;

	case AstValue::STRING:
		os << var.toString();
		break;
	}
	return os;
}

inline AstValue &AstValue::operator=(const AstValue &rhs) {
	if (this != &rhs) {
		if (type() != rhs.type() && type() == STRING) {
			delete var.string;
		}
		type_ = rhs.type();
		switch (type()) {
		case AstValue::INTEGER:
			var.integer = rhs.var.integer;
			break;

		case AstValue::REAL:
			var.real = rhs.var.real;
			break;

		case AstValue::STRING:
			var.string = new std::string(*rhs.var.string);
			break;
		}
	}
	return *this;
}

inline AstValue AstValue::operator+(const AstValue &rhs) {
	if (type() == REAL || rhs.type() == REAL) {
		return AstValue(toReal() + rhs.toReal());
	} else if (type() == INTEGER || rhs.type() == INTEGER) {
		return AstValue(toInt() + rhs.toInt());
	}
	return AstValue(toString() + rhs.toString());
}

inline AstValue AstValue::operator*(const AstValue &rhs) {
	if (type() == REAL || rhs.type() == REAL) {
		return AstValue(toReal() * rhs.toReal());
	} else if (type() == INTEGER && rhs.type() == INTEGER) {
		return AstValue(toInt() * rhs.toInt());
	}
	UNREACHABLE();
}

inline AstValue AstValue::operator/(const AstValue &rhs) {
	if (type() == REAL || rhs.type() == REAL) {
		return AstValue(toReal() / rhs.toReal());
	} else if (type() == INTEGER && rhs.type() == INTEGER) {
		return AstValue(toInt() / rhs.toInt());
	}
	UNREACHABLE();
}

inline AstValue AstValue::operator%(const AstValue &rhs) {
	if (type() == INTEGER && rhs.type() == INTEGER) {
		return AstValue(toInt() % rhs.toInt());
	}
	UNREACHABLE();
}

inline AstValue AstValue::operator^(const AstValue &rhs) {
	return AstValue(std::powf(toReal(), rhs.toReal()));
}

inline AstValue AstValue::operator==(const AstValue &rhs) {
	if (type() != STRING && rhs.type() != STRING) {
		return AstValue(toReal() == rhs.toReal());
	}
	return AstValue(toString() == rhs.toString());
}

inline AstValue AstValue::operator!=(const AstValue &rhs) {
	return !AstValue::operator==(rhs);
}

inline AstValue AstValue::operator<(const AstValue &rhs) {
	if (type() != STRING && rhs.type() != STRING) {
		return AstValue(toReal() < rhs.toReal());
	}
	return AstValue(toString() < rhs.toString());
}

inline AstValue AstValue::operator<=(const AstValue &rhs) {
	if (type() != STRING && rhs.type() != STRING) {
		return AstValue(toReal() <= rhs.toReal());
	}
	return AstValue(toString() <= rhs.toString());
}

inline AstValue AstValue::operator>(const AstValue &rhs) {
	return !operator<=(rhs);
}

inline AstValue AstValue::operator>=(const AstValue &rhs) {
	return !operator<(rhs);
}

inline AstValue AstValue::operator&&(const AstValue &rhs) {
	if (type() == REAL || rhs.type() == REAL) {
		return AstValue(toReal() && rhs.toReal());
	} else if (type() == INTEGER && rhs.type() == INTEGER) {
		return AstValue(toInt() && rhs.toInt());
	}
	UNREACHABLE();
}

inline AstValue AstValue::operator||(const AstValue &rhs) {
	if (type() == REAL || rhs.type() == REAL) {
		return AstValue(toReal() || rhs.toReal());
	} else if (type() == INTEGER && rhs.type() == INTEGER) {
		return AstValue(toInt() || rhs.toInt());
	}
	UNREACHABLE();
}

inline AstValue AstValue::operator!() {
	return AstValue(!operator bool());
}

inline AstValue::operator bool() {
	if (type() != STRING) {
		return (type() == INTEGER) ? toInt() : toReal();
	}
	UNREACHABLE();
}
