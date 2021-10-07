#ifndef STREAMCALC_CALC_HPP
#define STREAMCALC_CALC_HPP

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

#include "operation.hpp"

template <typename T>
class Calc
{
private:
	std::vector<std::unique_ptr<Operation<T>>> m_chain;
	std::vector<Value<T>> m_variables;
	
public:
	Calc(const std::string& str_source)
	{
		m_variables.push_back(Value<double>{ 0.0 });
		m_variables.push_back(Value<double>{ 0.0 });
		
		m_chain.push_back(std::unique_ptr<Operation<double>>{ new Unary<double>{ [](double a) -> double { return -a; }, m_variables[0] } });
		m_chain.push_back(std::unique_ptr<Operation<double>>{ new Binary<double>{ [](double a, double b) -> double { return a + b; }, *(m_chain[0]), m_variables[1] } });
	}
	
	int getNumberOfVariables() noexcept
	{
		return m_variables.size();
	}
	
	T getResult(const std::vector<T>& values)
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
	
};

#endif