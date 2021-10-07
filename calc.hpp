#ifndef STREAMCALC_CALC_HPP
#define STREAMCALC_CALC_HPP

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "operation.hpp"
#include "util.hpp"

// Длинные методы реализованы в данном файле, т.к. иначе пришлось бы явно указывать
// компилятору на необходимость разрешения темплейта с необходимыми типами
template <typename T>
class Calc
{
private:
	std::vector<std::shared_ptr<Operation<T>>> m_chain;
	std::vector<Value<T>> m_variables;
	
	void parse(const std::string& str_source);
	
public:
	Calc(const std::string& str_source)
	{
		parse(str_source);
	}
	
	void changeModel(const std::string& str_source)
	{
		m_chain.clear();
		m_variables.clear();
		parse(str_source);
	}
	
	int getNumberOfVariables() noexcept
	{
		return m_variables.size();
	}
	
	T getResult(const std::vector<T>& values);
};

template <typename T>
T Calc<T>::getResult(const std::vector<T>& values)
{
	if (values.size() != m_variables.size())
	{
		throw std::runtime_error("Wrong number of variables provided");
	}
	
	int i{ 0 };
	for(auto x : values)
	{
		m_variables[i].set(x);
		++i;
	}
	
	// Написано именно так, чтобы избегать StackOverflow из-за больших выражений
	// Вычисляет по пять операций за раз
	for(int i = 4; i < m_chain.size(); i += 5)
	{
		m_chain[i]->getResult();
	}
	
	return m_chain[m_chain.size() - 1]->getResult();
}

template <typename T>
void Calc<T>::parse(const std::string& str_source)
{
	using namespace parser_util;
	std::string expr{ str_source };
	
	for(char x : expr)
	{
		if (!(letter(x) || digit(x) || arithm(x)))
		switch(x)
		{
			case '(':
			case ')':
			case ' ':
			case '.':
			case ',':
			{
				break;
			}
			default:
			{
				throw std::runtime_error{ "Illegal symbols in formula" };
			}
		}
	}
	
	std::stringstream ss;
	
	std::string::size_type i{}, j{};
	
	T tmpTypeT{};
	auto str = expr;
	expr = "";
	for(i = 0; i < str.size(); i++)
	{
		if (letter(str[i]))
		{
			ss << str[i];
		}
		else if (digit(str[i]))
		{
			ss << str[i];
		}
		else if (arithm(str[i]) 
			|| (str[i] == ' ') 
			|| (str[i] == '(')
			|| (str[i] == ')')
			|| (str[i] == '%') 
			|| (str[i] == '#') 
			|| (str[i] == '$') 
			|| (str[i] == '@'))
		{
			auto size = ss.str().size();
			if (size != 0)
			if (checkFunction(ss.str()))
			{
				str.replace(i - size, size, getFuncCode(ss.str()));
				i = i - size;
				ss.str("");
				ss.clear();
			}
			else if (checkVariable(ss.str()))
			{
				expr = ss.str();
				
				ss.str("");
				ss.clear();
				
				m_variables.push_back(Value<T>{ 0 });
				ss << "@" << m_variables.size() - 1 << "@";
				
				std::string::size_type excl{ 0 };
				while((j = str.find(expr, excl)) != std::string::npos)
				{
					if ((j + size < str.size()) && (letter(str[j + size]) || digit(str[j + size])))
					{
						excl = j + 1;
						continue;
					}
					str.replace(j, size, ss.str());
				}
				
				i = i - size;
				ss.str("");
				ss.clear();
			}
			else
			{
				ss >> tmpTypeT;
				if (ss.fail())
				{
					throw std::runtime_error{ "Wrong syntax" };
				}
				
				m_chain.push_back(std::shared_ptr<Operation<T>>{ new Value<T>{ tmpTypeT } });
				
				ss.str("");
				ss.clear();
				ss << "$" << m_chain.size() - 1 << "$";
				
				str.replace(i - size, size, ss.str());
				
				i = i - size;
				ss.str("");
				ss.clear();
			}
			if (arithm(str[i]))
			{
				str.replace(i, 1, getArithmCode(str[i]));
			}
			if ((str[i] == '%')
			|| (str[i] == '#')
			|| (str[i] == '$')
			|| (str[i] == '@'))
			{
				i = str.find_first_of(str[i], i + 1);
			}
		}
		else
		{
			throw std::runtime_error{ "Unknown symbols found; something went wrong" };
		}
	}
	auto size = ss.str().size();
	if (size > 0)
	{
		if (checkFunction(ss.str()))
		{
			str.replace(i - size, size, getFuncCode(ss.str()));
			ss.str("");
			ss.clear();
		}
		else if (checkVariable(ss.str()))
		{
			expr = ss.str();
			
			ss.str("");
			ss.clear();
			
			m_variables.push_back(Value<T>{ 0 });
			ss << "@" << m_variables.size() - 1 << "@";
			
			std::string::size_type excl{ 0 };
			while((j = str.find(expr, excl)) != std::string::npos)
			{
				if ((j + size < str.size()) && (letter(str[j + size]) || digit(str[j + size])))
				{
					excl = j + 1;
					continue;
				}
				str.replace(j, size, ss.str());
			}
			
			ss.str("");
			ss.clear();
		}
		else
		{
			ss >> tmpTypeT;
			if (ss.fail())
			{
				throw std::runtime_error{ "Wrong syntax" };
			}
			
			m_chain.push_back(std::shared_ptr<Operation<T>>{ new Value<T>{ tmpTypeT } });
			
			ss.str("");
			ss.clear();
			ss << "$" << m_chain.size() - 1 << "$";
			
			str.replace(i - size, size, ss.str());
			
			ss.str("");
			ss.clear();
		}
	}
	
	std::cout << str << "\n";
	
	std::vector<std::string> list;
	
	while((i = str.find_last_of('(')) != std::string::npos)
	{
		j = str.find_first_of(')', i);
		if (j == std::string::npos)
		{
			throw std::runtime_error{ "Wrong syntax" };
		}
		if (j - i < 2)
		{
			throw std::runtime_error{ "Wrong syntax" };
		}
		list.push_back(str.substr(i + 1, j - i - 1));
		ss << "%" << list.size() - 1 << "%";
		str.replace(i, j - i + 1, ss.str());
		ss.str("");
	}
	
	list.push_back(str);
	
	std::cout << "\n";
	
	for(auto& s : list)
	{
		std::cout << s << "\n";
	}
	
	
}

#endif