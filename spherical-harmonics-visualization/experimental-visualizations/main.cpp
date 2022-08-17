#include "vvt_app.hpp"
// std
#include <stdlib.h>
#include <iostream>
#include <stdexcept>

int main()
{
	vae::VvtApp app{};
	try 
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		double test = sh::EvalSH(1, 1, { 1.0f,1.0f, 1.0f });
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}