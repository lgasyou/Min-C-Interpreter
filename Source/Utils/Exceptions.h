#pragma once
#include "Parsing\Token.h"
#include<cstdio>
#include<exception>

class Exception : public std::exception{
public:
	Exception(int line) 
	: std::exception(), exception_line_(line){}
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
			printf("解析出现错误\n错误信息：需要一个\"%s\"\n错误位置：Line:%d\n", rea_, line);
		}
		else if (type == Token::IDENTIFIER) {
			printf("解析出现错误\n错误信息：需要一个变量\n错误位置：Line:%d\n", line);
		}
		else {

			printf("解析出现错误\n错误信息：需要一个数据\n错误位置：Line:%d\n",line);
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
		printf("解析出现错误\n错误信息：语法错误\n错误位置：Line:%d\n", line);
	}
};

class OutException : public Exception {
public:
	OutException(int line)
		:Exception(line) {
		printf("解析出现错误\n错误信息：输出格式错误\n错误位置：Line:%d\n", line);
	}
};

class InException : public Exception {
public:
	InException(int line)
		:Exception(line) {
		printf("解析出现错误\n错误信息：输入格式错误\n错误位置：Line:%d\n", line);
	}
};

class OpException : public Exception {
public:
	OpException(int line)
		:Exception(line) {
		printf("解析出现错误\n错误信息：表达式错误\n错误位置：Line:%d\n", line);
	}
};

class FuncDecException : public Exception {
public:
	FuncDecException(int line)
		:Exception(line) {
		printf("解析出现错误\n错误信息：函数声明错误\n错误位置：Line:%d\n", line);
	}
};

class IdenDecException : public Exception{
public:
	IdenDecException(int line)
		:Exception(line){
		printf("解析出现错误\n错误信息：变量声明错误\n错误位置：Line:%d\n", line);
	}
};

class ScanException : public Exception {
public:
	ScanException(int line)
		:Exception(line) {
		printf("解析出现错误\n错误信息：非法字符\n错误位置：Line:%d\n", line);
	}
};