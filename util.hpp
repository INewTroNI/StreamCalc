#ifndef STREAMCALC_PARSER_UTIL_HPP
#define STREAMCALC_PARSER_UTIL_HPP

#include <string>

namespace parser_util
{
	bool letter(const char c);
	bool digit(const char c);
	bool arithm(const char c);
	bool unary(const char c);
	bool binary(const char c);
	bool precedence(const char c);
	bool checkFunction(const std::string& str);
	bool checkVariable(const std::string& str);
	std::string getFuncCode(const std::string& str);
	std::string getArithmCode(const char c);
}

#endif