#ifndef STREAMCALC_OPERATION_HPP
#define STREAMCALC_OPERATION_HPP

#include <functional>
#include <memory>

template <typename T>
class Operation
{
protected:
	// Данные два поля были добавлены, чтобы операция выполнялась лишь 1 раз на одних данных
	// Это также необходимо для избежания StackOverflow при разрешении (см. Calc и реализацию Unary/Binary)
	T m_result;
	bool m_computed{ false };
	
public:
	Operation() = default;
	Operation(T result) : m_result{ result } {}
	
	virtual ~Operation() = default;
	
	virtual T getResult() = 0;
	
	void reset() { m_computed = false; }
};

// Данный класс необходим для того, чтобы все операции в итоге указывали на него и таким образом могли разрешиться
template <typename T>
class Value final : public Operation<T>
{
public:
	Value(T result) : Operation<T>{ result } {}
	
	T set(T result) { return ((this->m_result) = result); }
	T getResult() override { return this->m_result; }
};

template <typename T>
class Unary final : public Operation<T>
{
protected:
	std::shared_ptr<Operation<T>> m_operand{};
	std::function<T(T)> m_function;
	
public:
	Unary(std::function<T(T)> fnc, std::shared_ptr<Operation<T>> operand) 
		: m_function{ fnc }, m_operand{ operand } 
	{}
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = m_function(m_operand->getResult());
		}
		
		return this->m_result;
	}
};

template <typename T>
class Binary final : public Operation<T>
{
protected:
	std::shared_ptr<Operation<T>> m_operandA{};
	std::shared_ptr<Operation<T>> m_operandB{};
	std::function<T(T, T)> m_function;
	
public:
	Binary(std::function<T(T, T)> fcn
	, std::shared_ptr<Operation<T>> operA
	, std::shared_ptr<Operation<T>> operB)
		: m_function{ fcn }, m_operandA{ operA }, m_operandB{ operB }
	{}
	
	T getResult() override
	{
		if (!this->m_computed)
		{
			this->m_result = m_function(m_operandA->getResult(), m_operandB->getResult());
		}
		
		return this->m_result;
	}
};

#endif