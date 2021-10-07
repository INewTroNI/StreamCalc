#include "main.hpp"

int main(int argc, char** argv)
{
	try
	{
		Calc<double> model{"-lg(x+10)*(sin(cos(y))+tan(5)) + 78,93 + lgn + lgnn"};
	}
	catch(std::runtime_error e)
	{
		std::cout << "Error: " << e.what() << "\n";
	}
	
	return 0;
}