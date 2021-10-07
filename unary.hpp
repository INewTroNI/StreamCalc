#ifndef STREAMCALC_OPERATION_UNARY_HPP
#define STREAMCALC_OPERATION_UNARY_HPP

#include "operation.hpp"

template <typename T>
class Unary : public Operation<T>
{
protected:
	Operation<T>& m_operand{};
	
public:
	Unary(Operation<T>& operand) : m_operand{ operand } {}
	
	Unary(Unary<T>& u) : m_operand{ u.m_operand } {}
	
	// Из-за использование стандартных референсов присвоение не будет возможно выполнить
	Unary<T>& operator=(const Unary<T>& u) = delete;
};

template <typename T>
class Negation final : public Unary<T>
{
public:
	Negation(Operation<T>& operand) : Unary<T>{ operand } {}
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = -(this->m_operand.getResult());
		}
		
		return this->m_result;
	}
};

#endif