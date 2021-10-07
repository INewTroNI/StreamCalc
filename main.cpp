#include "main.hpp"

int main(int argc, char** argv)
{
	Calc<double> model{"test"};
	
	std::vector<double> v{ 5.0, 3.0 };
	
	std::cout << model.getResult(v);
	
	return 0;
}