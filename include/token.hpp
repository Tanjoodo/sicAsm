#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

struct token {
	std::string value;
	token() { }
	token(std::string value)
		:value(value) { }
};

#endif
