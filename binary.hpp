#ifndef STREAMCALC_OPERATION_BINARY_HPP
#define STREAMCALC_OPERATION_BINARY_HPP

#include "operation.hpp"

template <typename T>
class Binary : public Operation<T>
{
protected:
	Operation<T>& m_operandA{};
	Operation<T>& m_operandB{};
	
public:
	Binary(Operation<T>& operA, Operation<T>& operB)
		: m_operandA{ operA }, m_operandB{ operB }
	{}
	
	Binary(Binary<T>& b) : m_operandA{ b.m_operandA }, m_operandB{ b.m_operandB } {}
	
	Binary& operator=(const Binary& b) = delete;
};

template <typename T>
class Addition : public Binary<T>
{
public:
	Addition(Operation<T>& a, Operation<T>& b) : Binary<T>{ a, b} {}
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = (this->m_operandA.getResult()) + (this->m_operandB.getResult());
		}
		
		return this->m_result;
	}
};

#endif