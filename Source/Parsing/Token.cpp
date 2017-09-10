#include "Token.h"
#include <algorithm>
#include <string>

#define T(name, string, precedence) #name,
const char* const Token::names_[NUM_TOKENS] = { TOKEN_LIST(T, T) };
#undef T


#define T(name, string, precedence) string,
const char* const Token::strings_[NUM_TOKENS] = { TOKEN_LIST(T, T) };
#undef T

#define T(name, string, precedence) precedence,
const int8_t Token::precedences_[NUM_TOKENS] = { TOKEN_LIST(T, T) };
#undef T


Token::Token(Type t, int pos, const std::string &v)
	: type(t), line(pos) {
	value = String(t) ? String(t) : v;
}

// static
Token::Type Token::GetType(const std::string &string) {
	int current = -1;
	std::find_if(std::cbegin(strings_), std::cend(strings_), [&](const char *s) {
		current += 1;
		if (!s) return false;
		return strcmp(string.c_str(), s) == 0;
	});
	Token::Type value = static_cast<Token::Type>(current);
	return Includes(value) ? Token::Type(value) : IDENTIFIER;
}

// static
bool Token::Includes(const std::string &string) {
	auto type = GetType(string);
	return type != IDENTIFIER;
}

// static
bool Token::Includes(Token::Type value) {
	return value != NUM_TOKENS - 1;
}


#include "Utils/UnitTest.h"
TEST_CASE(Token) {
	ASSERT_EQ(Token::IDENTIFIER, Token::GetType("i"));
	ASSERT_EQ(Token::INT, Token::GetType("int"));
	ASSERT_EQ(Token::REAL, Token::GetType("real"));
}
