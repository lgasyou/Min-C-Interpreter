#pragma once
#include "Parsing\Token.h"
#include<cstdio>
#include<exception>

class Exception : public std::exception{
public:
	Exception(int line) 
	: exception_line_(line){}
private:
	int exception_line_;
};

class EatException : public Exception {
public:
	EatException(int line, Token::Type type) 
		: Exception(line), need_type_(type){
		if (type != Token::STRING_LITERAL 
			&& type != Token::INTEGER_LITERAL 
			&& type != Token::REAL_LITERAL
			&& type != Token::IDENTIFIER) {
			rea_ = token.String(type);
			printf("�������ִ���\n������Ϣ����Ҫһ��\"%s\"\n����λ�ã�Line:%d\n", rea_, line);
		}
		else if (type == Token::IDENTIFIER) {
			printf("�������ִ���\n������Ϣ����Ҫһ������\n����λ�ã�Line:%d\n", line);
		}
		else {

			printf("�������ִ���\n������Ϣ����Ҫһ������\n����λ�ã�Line:%d\n",line);
		}
		
	}
private:
	Token token;
	Token::Type	need_type_;
	const char* rea_;
};

class StatementException : public Exception{
public:
	StatementException(int line)
		: Exception(line){
		printf("�������ִ���\n������Ϣ���﷨����\n����λ�ã�Line:%d\n", line);
	}
};

class OutException : public Exception {
public:
	OutException(int line)
		:Exception(line) {
		printf("�������ִ���\n������Ϣ�������ʽ����\n����λ�ã�Line:%d\n", line);
	}
};

class InException : public Exception {
public:
	InException(int line)
		:Exception(line) {
		printf("�������ִ���\n������Ϣ�������ʽ����\n����λ�ã�Line:%d\n", line);
	}
};

class OpException : public Exception {
public:
	OpException(int line)
		:Exception(line) {
		printf("�������ִ���\n������Ϣ�����ʽ����\n����λ�ã�Line:%d\n", line);
	}
};

class FuncDecException : public Exception {
public:
	FuncDecException(int line)
		:Exception(line) {
		printf("�������ִ���\n������Ϣ��������������\n����λ�ã�Line:%d\n", line);
	}
};
