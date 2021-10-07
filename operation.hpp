#ifndef STREAMCALC_OPERATION_HPP
#define STREAMCALC_OPERATION_HPP

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

#include "unary.hpp"
#include "binary.hpp"

#endif