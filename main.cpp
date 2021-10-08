#include "main.hpp"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: ";
		return 0;
	}
	try
	{
		Calc<double> model{ argv[1] };
		
		std::cout << model.getNumberOfVariables() << "\n";
		
		std::vector<double> v;
		
		double tmp;
		
		for (int i = 0; i < model.getNumberOfVariables(); i++)
		{
			std::cin >> tmp;
			v.push_back(tmp);
		}
		
		std::cout << model.getResult(v);
	}
	catch(std::runtime_error e)
	{
		std::cout << "Error: " << e.what() << "\n";
	}
	
	return 0;
}