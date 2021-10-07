#include "main.hpp"

int main(int argc, char** argv)
{
	std::vector<std::unique_ptr<Operation<double>>> chain;
	std::vector<Value<double>> values;
	
	values.push_back(Value<double>{ 0.0 });
	values.push_back(Value<double>{ 0.0 });
	
	chain.push_back(std::unique_ptr<Operation<double>>{ new Unary<double>{ [](double a) -> double { return -a; }, values[0] } });
	chain.push_back(std::unique_ptr<Operation<double>>{ new Binary<double>{ [](double a, double b) -> double { return a + b; }, *(chain[0]), values[1] } });
	
	values[0].setValue(5);
	values[1].setValue(3);
	
	std::cout << chain[1]->getResult();
	
	return 0;
}