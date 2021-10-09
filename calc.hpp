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
	
	// Во всех функциях парсера если аргумент не константный, то он может быть изменён функцией
	
	// Меняет все символы в нижний регистр, заменяет ',' на '.' и проверяет строку на наличие запрещённых символов
	// выполняет частичную проверку синтаксиса
	void parserReadyString(std::string& expr);
	
	// Заменяет все синтаксические струткуры (операторы, переменные, значения) на соответствующие токены
	// выполняет частичную проверку синтаксиса
	void parserTokenizeString(std::string& str);
	
	// Разбивает формулу на подвыражения в соответствии со скобками
	// выполняет частичную проверку синтаксиса
	std::vector<std::string> parserSplit(std::string& str);
	
	// Читает номер операнда из строки начиная с положения i; читает влево, i изменяется
	int parserReadOperandN(const std::string& str, int& i);
	
	// Добавляет бинарную операцию в m_chain; референсы не const т.к. передаются в конструктор, который нарушил бы const
	void parserPushBinary(char sign, std::shared_ptr<Operation<T>>& a, std::shared_ptr<Operation<T>>& b);
	
	// Добавляет унарную операцию в m_chain; референсы не const т.к. передаются в конструктор, который нарушил бы const
	void parserPushUnary(char sign, std::shared_ptr<Operation<T>>& a);
	
public:
	Calc(const std::string& str_source)
	{
		try
		{
			parse(str_source);
		}
		catch(...)
		{
			m_chain.clear();
			m_variables.clear();
			throw;
		}
		
	}
	
	void changeModel(const std::string& str_source)
	{
		m_chain.clear();
		m_variables.clear();
		try
		{
			parse(str_source);
		}
		catch(...)
		{
			m_chain.clear();
			m_variables.clear();
			throw;
		}
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
void Calc<T>::parserReadyString(std::string& expr)
{
	using namespace parser_util;
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
	
	return;
}

template <typename T>
void Calc<T>::parserTokenizeString(std::string& str)
{
	using namespace parser_util;
	std::stringstream ss;
	std::string::size_type i{}, j{};
	
	T tmpTypeT{};
	std::string expr;
	
	for(i = 0; i <= str.size(); i++)
	{
		if (i < str.size() && (letter(str[i]) || digit(str[i]) || str[i] == '.'))
		{
			ss << str[i];
		}
		else /* if (arithm(str[i]) 
			|| (str[i] == ' ') 
			|| (str[i] == '(')
			|| (str[i] == ')')
			|| (str[i] == '%') 
			|| (str[i] == '#') 
			|| (str[i] == '$') 
			|| (str[i] == '@')) */ 
			// also when i = str.size() (to not duplicate code; it's to check if ss is empty after end of string)
		{
			auto size = ss.str().size();
			
			if (size != 0)
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
			
			i = i - size;
			
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
	}
	
	return;
}

template <typename T>
std::vector<std::string> Calc<T>::parserSplit(std::string& str)
{
	std::vector<std::string> list;
	std::stringstream ss;
	
	std::string::size_type i{}, j{};
	
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
		ss.clear();
	}
	
	list.push_back(str);
	
	return list;
}

template <typename T>
int Calc<T>::parserReadOperandN(const std::string& str, int& i)
{	
	using namespace parser_util;
	std::stringstream ss;
	std::string expr;
	int opN;
	
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
	
	ss >> opN;
	
	if (ss.fail())
	{
		throw std::runtime_error{ "Something went wrong" };
	}
	
	return opN;
}

template <typename T>
void Calc<T>::parserPushBinary(char sign, std::shared_ptr<Operation<T>>& a, std::shared_ptr<Operation<T>>& b)
{
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
					,a
					,b
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
					,a
					,b
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
					,a
					,b
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
					,a
					,b
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
					,a
					,b
				}
			});
			break;
		}
		default:
			throw std::runtime_error{ "Wrong syntax: unknown operation" };
	}
	
	return;
}

template <typename T>
void Calc<T>::parserPushUnary(char sign, std::shared_ptr<Operation<T>>& a)
{
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
					,a
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
					,a
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
					,a
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
					,a
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
					,a
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
					,a
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
					,a
				}
			});
			break;
		}
		default:
			throw std::runtime_error{ "Wrong syntax: unknown operation" };
	}
	
	return;
}

template <typename T>
void Calc<T>::parse(const std::string& str_source)
{
	using namespace parser_util;
	std::string expr{ str_source };
	
	parserReadyString(expr);
	parserTokenizeString(expr);
	auto list = parserSplit(expr);
	
	std::stringstream ss;
	std::string::size_type j{};
	
	std::shared_ptr<Operation<T>> opLeft{ nullptr };
	std::shared_ptr<Operation<T>> opRight{ nullptr };
	
	int opStart{ -1 };
	int opN{ 0 };
	char sign{ 0 };
	
	for(int k = 0; k < list.size(); ++k)
	{
		auto& str{ list[k] };
		for (int precedenceLevel = 0; precedenceLevel < 4; ++precedenceLevel)
		{
			opRight = nullptr;
			opStart = -1;
			
			for(int i = str.size() - 1; i >= 0; --i)
			{
				// std::cout << str << " " << str[i-1] << str[i] << " " << i << "\n";
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
							
							opN = parserReadOperandN(str, i);
							
							if (str[i] == '@')
							{
								opRight = m_variables[opN];
							}
							else
							{
								opRight = m_chain[opN];
							}
						}
						else
						{
							if (sign == 0)
							{
								throw std::runtime_error("Wrong syntax: operands without operation");
							}
							
							opN = parserReadOperandN(str, i);
							
							if (str[i] == '@')
							{
								opLeft = m_variables[opN];
							}
							else
							{
								opLeft = m_chain[opN];
							}
							
							parserPushBinary(sign, opLeft, opRight);
							
							opN = m_chain.size() - 1;
							opRight = m_chain[opN];
							opLeft = nullptr;
							
							ss << "$" << opN << "$";
							
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
						
						sign = str[i - 1];
						
						i = i - 2;
						
						if ((precedence(sign) != precedenceLevel) && !(sign == '-'))
						{
							opRight = nullptr;
							sign = 0;
							opStart = -1;
							break;
						}
						
						if (sign == '-')
						{
							int j;
							for(j = i - 1; str[j] >= 0 && str[j] == ' '; --j);
							if ((j > -1) && (str[j] == '$' || str[j] == '@'))
							{
								// Когда мы встретили бинарный минус
								if (precedence(sign) == precedenceLevel)
								{
									// Когда можем разрешать бинарный минус
									break;
								}
								else
								{
									opRight = nullptr;
									sign = 0;
									opStart = -1;
									break;
								}
							}
							else if ((j > 0) && (precedence(str[j - 1]) != precedenceLevel))
							{
								// When left operation doesn't have current precedence level
								// that means that we won't resolve it next and unary minus can be resolved later
								opRight = nullptr;
								sign = 0;
								opStart = -1;
								break;
							}
							// When left operation has current precedence level
							// that means that we need to resolve it next and unary minus must be resolved now
							// it also means that all right operations (if any) are of lower precedence than it 
							// and we can safely resolve current unary minus with curren right operand
						}
						
						if (opRight == nullptr)
						{
							throw std::runtime_error{ "Wrong syntax: operation without right operand" };
						}
						
						if (unary(sign))
						{
							parserPushUnary(sign, opRight);
							
							opN = m_chain.size() - 1;
							opRight = m_chain[opN];
							
							ss << "$" << opN << "$";
							
							str.replace(i, opStart - i + 1, ss.str());
							
							opStart = i + ss.str().size() - 1;
							
							ss.str("");
							ss.clear();
							
							sign = 0;
						}
						
						break;
					}
					/* case '%':
						while (str[--i] != '%'); */
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
		}
		if (opRight != nullptr)
		{
			ss << "%" << k << "%";
			expr = ss.str();
			ss.str("");
			ss.clear();
			if (m_chain.size() > opN && m_chain[opN] == opRight)
			{
				ss << "$" << opN << "$";
			}
			else if (m_variables.size() > opN && m_variables[opN] == opRight)
			{
				ss << "@" << opN << "@";
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
	}
	
	if (m_chain.size() == 0 && m_variables.size() == 1)
	{
		m_chain.push_back(m_variables[0]);
	}
}

#endif