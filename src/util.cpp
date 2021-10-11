#include "util.hpp"

bool parser_util::letter(const char c)
{
	return ((c >= 'a') && (c <= 'z') || (c >= 'A') && (c <= 'Z'));
}

bool parser_util::digit(const char c)
{
	return ((c >= '0') && (c <= '9'));
}

bool parser_util::arithm(const char c)
{
	return ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '^'));
}

bool parser_util::unary(const char c)
{
	return ((c == '-') || (c == 'g') || (c == 'n') || (c == 's') || (c == 'c') || (c == 't') || (c == 'q'));
}

bool parser_util::binary(const char c)
{
	return ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '^'));
}

int parser_util::precedence(const char c)
{
	if (unary(c) && !(c == '-'))
	{
		return 0;
	}
	switch(c)
	{
		case '^':
			return 1;
		case '*':
		case '/':
			return 2;
		case '+':
		case '-':
			return 3;
	}
	return 4;
}

bool parser_util::checkFunction(const std::string& str)
{
	return (str == "lg" 
	|| str == "ln"
	|| str == "sin"
	|| str == "cos"
	|| str == "tan"
	|| str == "sqrt");
}

bool parser_util::checkVariable(const std::string& str)
{
	for(int i = 0; i < str.size(); i++)
	{
		if (i == 0)
		{
			if (!letter(str[i]))
			{
				return false;
			}
		}
		else
		{
			if (!(letter(str[i]) || digit(str[i])))
			{
				return false;
			}
		}
	}
	
	return true;
}

bool parser_util::checkNumber(const std::string& str)
{
	bool comma{false};
	for(int i = 0; i < str.size(); i++)
	{
		if (i == 0)
		{
			if (!digit(str[i]))
			{
				return false;
			}
		}
		else
		{
			if (!(digit(str[i]) || (str[i] == '.' && !comma)))
			{
				return false;
			}
			else if (str[i] == '.')
			{
				comma = true;
			}
		}
	}
	
	return true;
}

std::string parser_util::getFuncCode(const std::string& str)
{
	if (str == "lg")
	{
		return "#g#";
	}
	if (str == "ln")
	{
		return "#n#";
	}
	if (str == "sin")
	{
		return "#s#";
	}
	if (str == "cos")
	{
		return "#c#";
	}
	if (str == "tan")
	{
		return "#t#";
	}
	if (str == "sqrt")
	{
		return "#q#";
	}
	
	return "#g#";
}

std::string parser_util::getArithmCode(const char c)
{
	switch(c)
	{
		case '-':
			return "#-#";
		case '+':
			return "#+#";
		case '*':
			return "#*#";
		case '/':
			return "#/#";
		case '^':
			return "#^#";
	}
	
	return "#-#";
}

