#include "main.hpp"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: \n";
		std::cout << argv[0] << " \"<formula>\"\n";
		std::cout << "If there are variables in the formula, they will be accepted in stdin stream one set per line.";
		return 0;
	}
	try
	{
		Calc<double> model{ argv[1] };
		
		// std::cout << model.getNumberOfVariables() << "\n";
		
		std::vector<double> v;
		double tmp;
		
		if (model.getNumberOfVariables() > 0)
		{
			while(!std::cin.eof())
			{
				tmp = 0;
				
				for (int i = 0; i < model.getNumberOfVariables(); i++)
				{
					if (!(std::cin >> tmp))
					{
						return 0;
					}
					
					v.push_back(tmp);
				}
				
				std::cout << model.getResult(v) << "\n";
				
				v.clear();
			}
		}
		else
		{
			std::cout << model.getResult(v) << "\n";
		}
	}
	catch(std::runtime_error e)
	{
		std::cout << "Error: " << e.what() << "\n";
	}
	
	return 0;
}