#ifndef STREAMCALC_OPERATION_HPP
#define STREAMCALC_OPERATION_HPP

#include <functional>

template <typename T>
class Operation
{
protected:
	T m_result;
	bool m_computed{ false };
	
public:
	Operation() = default;
	Operation(T result) : m_result{ result } {}
	
	virtual ~Operation() = default;
	
	virtual T getResult() = 0;
	
	void reset() { m_computed = false; }
};

template <typename T>
class Value final : public Operation<T>
{
public:
	Value(T result) : Operation<T>{ result } {}
	
	T setValue(T result) { return this->m_result = result; }
	T getResult() override { return this->m_result; }
};

template <typename T>
class Unary final : public Operation<T>
{
protected:
	Operation<T>& m_operand{};
	std::function<T(T)> m_function;
	
public:
	Unary(std::function<T(T)> fnc, Operation<T>& operand) 
		: m_function{ fnc }, m_operand{ operand } 
	{}
	
	Unary(Unary<T>& u) : m_function{ u.m_function }, m_operand{ u.m_operand } {}
	
	// Из-за использование стандартных референсов присвоение не будет возможно выполнить
	Unary<T>& operator=(const Unary<T>& u) = delete;
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = m_function(m_operand.getResult());
		}
		
		return this->m_result;
	}
};

template <typename T>
class Binary final : public Operation<T>
{
protected:
	Operation<T>& m_operandA{};
	Operation<T>& m_operandB{};
	std::function<T(T, T)> m_function;
	
public:
	Binary(std::function<T(T, T)> fcn, Operation<T>& operA, Operation<T>& operB)
		: m_function{ fcn }, m_operandA{ operA }, m_operandB{ operB }
	{}
	
	Binary(Binary<T>& b) 
		: m_function{ b.m_function }
		, m_operandA{ b.m_operandA }
		, m_operandB{ b.m_operandB } 
	{}
	
	Binary& operator=(const Binary& b) = delete;
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = m_function(m_operandA.getResult(), m_operandB.getResult());
		}
		
		return this->m_result;
	}
};

#endif