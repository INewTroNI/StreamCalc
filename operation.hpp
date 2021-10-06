#ifndef STREAMCALC_OPERATION_HPP
#define STREAMCALC_OPERATION_HPP

template <typename T>
class Operation
{
protected:
	T m_result;
	bool computed{ false };
	
public:
	Operation() = default;
	Operation(T v) : m_result{ v } {}
	
	virtual ~Operation() = default;
	
	virtual T getResult() const = 0;
	
	void reset() { computed = false; }
};

template <typename T>
class Value final : public Operation<T>
{
public:
	Value(T v) : Operation{ v } {}
	
	virtual T getResult() { return this.m_v; }
};

#include "unary.hpp"
#include "binary.hpp"

#endif