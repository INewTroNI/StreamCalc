#ifndef STREAMCALC_CALC_HPP
#define STREAMCALC_CALC_HPP

#include <algorithm>
#include <cctype>
#include <cmath>
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
	std::vector<std::shared_ptr<Value<T>>> m_variables;
	
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
		m_variables[i]->set(x);
		++i;
	}
	
	// Написано именно так, чтобы избегать StackOverflow из-за больших выражений
	// Вычисляет по пять операций за раз
	for(int i = 4; i < m_chain.size(); i += 5)
	{
		m_chain[i]->getResult();
	}
	
	if (m_chain.size() > 0)
	{
		return m_chain[m_chain.size() - 1]->getResult();
	}
	
	return 0;
}

template <typename T>
void Calc<T>::parse(const std::string& str_source)
{
	using namespace parser_util;
	std::string expr{ str_source };
	
	int leftBracketN{ 0 };
	int rightBracketN{ 0 };
	
	for(char& x : expr)
	{
		if (!(letter(x) || digit(x) || arithm(x) || x == ','))
		{
			switch(x)
			{
				case '(':
				{
					++leftBracketN;
					break;
				}
				case ')':
				{
					++rightBracketN;
					break;
				}
				case ' ':
				case '.':
				{
					break;
				}
				default:
				{
					throw std::runtime_error{ "Illegal symbols in formula" };
				}
			}
		}
		else if (x == ',')
		{
			x = '.';
		}
		else
		{
			x = std::tolower(x);
		}
	}
	
	if (rightBracketN != leftBracketN)
	{
		throw std::runtime_error{ "Wrong syntax: number of ( and ) doesn't match" };
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
		else if (digit(str[i]) || str[i] == '.')
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
				
				m_variables.push_back(std::shared_ptr<Value<T>>{ new Value<T>{ 0 } });
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
			else if (checkNumber(ss.str()))
			{
				ss >> tmpTypeT;
				if (ss.fail())
				{
					throw std::runtime_error{ "Something went wrong" };
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
			else
			{
				throw std::runtime_error{ "Wrong syntax" };
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
			
			m_variables.push_back(std::shared_ptr<Value<T>>{ new Value<T>{ 0 } });
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
		else if (checkNumber(ss.str()))
		{
			ss >> tmpTypeT;
			if (ss.fail())
			{
				throw std::runtime_error{ "Something went wrong" };
			}
			
			m_chain.push_back(std::shared_ptr<Operation<T>>{ new Value<T>{ tmpTypeT } });
			
			ss.str("");
			ss.clear();
			ss << "$" << m_chain.size() - 1 << "$";
			
			str.replace(i - size, size, ss.str());
			
			ss.str("");
			ss.clear();
		}
		else
		{
			throw std::runtime_error{ "Wrong syntax" };
		}
	}
	
	std::vector<std::string> list;
	
	while((i = str.find_last_of('(')) != std::string::npos)
	{
		j = str.find_first_of(')', i);
		if (j == std::string::npos)
		{
			throw std::runtime_error{ "Wrong syntax: there are unclosed ()" };
		}
		if (j - i < 2)
		{
			throw std::runtime_error{ "Wrong syntax: empty () found" };
		}
		list.push_back(str.substr(i + 1, j - i - 1));
		ss << "%" << list.size() - 1 << "%";
		str.replace(i, j - i + 1, ss.str());
		ss.str("");
	}
	
	list.push_back(str);
	
	std::shared_ptr<Operation<T>> opLeft{ nullptr };
	std::shared_ptr<Operation<T>> opRight{ nullptr };
	int opStart{ -1 };
	int tmpInt{ 0 };
	char sign{ 0 };
	for(int k = 0; k < list.size(); ++k)
	{
		auto& str{ list[k] };
		for (int precedenceLevel = 0; precedenceLevel < 4; ++precedenceLevel)
		{
			for(int i = str.size() - 1; i >= 0; --i)
			{
				switch(str[i])
				{
					case '$':
					case '@':
					{
						if (opRight == nullptr)
						{
							if (opStart < 0)
							{
								opStart = i;
							}
							
							--i;
							while(digit(str[i]))
							{
								ss << str[i];
								--i;
							}
							
							if (ss.str().size() == 0)
							{
								throw std::runtime_error{ "Something went wrong" };
							}
							
							expr = ss.str();
							std::reverse(expr.begin(), expr.end());
							ss.str(expr);
							
							ss >> tmpInt;
							
							if (ss.fail())
							{
								throw std::runtime_error{ "Something went wrong" };
							}
							
							ss.str("");
							ss.clear();
							
							if (str[i] == '@')
							{
								if (m_variables.size() > tmpInt)
								{
									opRight = m_variables[tmpInt];
								}
								else
								{
									throw std::runtime_error{ "Something went wrong" };
								}
							}
							else
							{
								if (m_chain.size() > tmpInt)
								{
									opRight = m_chain[tmpInt];
								}
								else
								{
									throw std::runtime_error{ "Something went wrong" };
								}
							}
						}
						else
						{
							if (sign == 0)
							{
								throw std::runtime_error("Wrong syntax: operands without operation");
							}
							
							int tmpStart{ i };
							
							--i;
							while(digit(str[i]))
							{
								ss << str[i];
								--i;
							}
							
							if (ss.str().size() == 0)
							{
								throw std::runtime_error{ "Something went wrong" };
							}
							
							expr = ss.str();
							std::reverse(expr.begin(), expr.end());
							ss.str(expr);
							
							ss >> tmpInt;
							
							if (ss.fail())
							{
								throw std::runtime_error{ "Something went wrong" };
							}
							
							ss.str("");
							ss.clear();
							
							if (str[i] == '@')
							{
								if (m_variables.size() > tmpInt)
								{
									opLeft = m_variables[tmpInt];
								}
								else
								{
									throw std::runtime_error{ "Something went wrong" };
								}
							}
							else
							{
								if (m_chain.size() > tmpInt)
								{
									opLeft = m_chain[tmpInt];
								}
								else
								{
									throw std::runtime_error{ "Something went wrong" };
								}
							}
							
							switch(sign)
							{
								case '-':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Binary<T>{
											[](T a, T b) -> T
											{
												return a - b;
											}
											,opLeft
											,opRight
										}
									});
									break;
								}
								case '+':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Binary<T>{
											[](T a, T b) -> T
											{
												return a + b;
											}
											,opLeft
											,opRight
										}
									});
									break;
								}
								case '*':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Binary<T>{
											[](T a, T b) -> T
											{
												return a * b;
											}
											,opLeft
											,opRight
										}
									});
									break;
								}
								case '/':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Binary<T>{
											[](T a, T b) -> T
											{
												return a / b;
											}
											,opLeft
											,opRight
										}
									});
									break;
								}
								case '^':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Binary<T>{
											[](T a, T b) -> T
											{
												return std::pow(a, b);
											}
											,opLeft
											,opRight
										}
									});
									break;
								}
								default:
									throw std::runtime_error{ "Wrong syntax: unknown operation" };
							}
							
							tmpInt = m_chain.size() - 1;
							opRight = m_chain[tmpInt];
							opLeft = nullptr;
							
							ss << "$" << tmpInt << "$";
							
							str.replace(i, opStart - i + 1, ss.str());
							
							opStart = i + ss.str().size() - 1;
							
							ss.str("");
							ss.clear();
							
							sign = 0;
						}
						break;
					}
					case '#':
					{
						if (sign != 0)
						{
							throw std::runtime_error{ "Wrong syntax: consecutive non-unary operations" };
						}
						
						if (opRight == nullptr)
						{
							throw std::runtime_error{ "Wrong syntax: operation without right operand" };
						}
						
						sign = str[i - 1];
						
						i = i - 2;
						
						if ((precedence(sign) != precedenceLevel) && !(sign == '-' && precedenceLevel == 3))
						{
							opRight = nullptr;
							sign = 0;
							opStart = -1;
							break;
						}
						
						if (precedenceLevel == 0)
						{
							if (sign == '-')
							{
								int j;
								for(j = i - 1; str[j] >= 0 && str[j] == ' '; --j);
								if ((j > -1) && (str[j] == '$' || str[j] == '@'))
								{
									opRight = nullptr;
									sign = 0;
									opStart = -1;
									break;
								}
							}
							switch(sign)
							{
								case '-':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return -a;
											}
											,opRight
										}
									});
									break;
								}
								case 'c':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::cos(a);
											}
											,opRight
										}
									});
									break;
								}
								case 's':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::sin(a);
											}
											,opRight
										}
									});
									break;
								}
								case 't':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::tan(a);
											}
											,opRight
										}
									});
									break;
								}
								case 'n':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::log(a);
											}
											,opRight
										}
									});
									break;
								}
								case 'g':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::log10(a);
											}
											,opRight
										}
									});
									break;
								}
								case 'q':
								{
									m_chain.push_back(std::shared_ptr<Operation<T>>{
										new Unary<T>{
											[](T a) -> T
											{
												return std::sqrt(a);
											}
											,opRight
										}
									});
									break;
								}
								default:
									throw std::runtime_error{ "Wrong syntax: unknown operation" };
							}
							
							tmpInt = m_chain.size() - 1;
							opRight = m_chain[tmpInt];
							
							ss << "$" << tmpInt << "$";
							
							str.replace(i, opStart - i + 1, ss.str());
							
							opStart = i + ss.str().size() - 1;
							
							ss.str("");
							ss.clear();
							
							sign = 0;
						}
						
						break;
					}
					case '%':
						while (str[--i] != '%');
					case ' ':
						break;
					default:
						throw std::runtime_error{ "Unexpected symbol; something went wrong" };
				}
			}
			
			
			if (sign != 0)
			{
				throw std::runtime_error{ "Wrong syntax: odd operator found" };
			}
			if (opRight != nullptr)
			{
				if (precedenceLevel == 3)
				{
					ss << "%" << k << "%";
					expr = ss.str();
					ss.str("");
					ss.clear();
					if (m_chain.size() > tmpInt && m_chain[tmpInt] == opRight)
					{
						ss << "$" << tmpInt << "$";
					}
					else if (m_variables.size() > tmpInt && m_variables[tmpInt] == opRight)
					{
						ss << "@" << tmpInt << "@";
					}
					else
					{
						throw std::runtime_error{ "Something went wrong" };
					}
					for(auto& x : list)
					{
						while((j = x.find(expr)) != std::string::npos)
						{
							x.replace(j, expr.size(), ss.str());
						}
					}
					ss.str("");
					ss.clear();
				}
				opRight = nullptr;
			}
			opStart = -1;
		}
		
	}
	
	if (m_chain.size() == 0 && m_variables.size() == 1)
	{
		m_chain.push_back(m_variables[0]);
	}
}

#endif