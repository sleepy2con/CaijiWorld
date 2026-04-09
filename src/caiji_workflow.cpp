
//#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
//#include "spdlog/fmt/ostr.h"  // support for user defined types

#include "world/world.h"
#include "constant/constant.h"
#include "spdlog/spdlog.h" 
#include <iostream>       


int main(int argc, char* argv[])
{
	spdlog::set_level(spdlog::level::debug);
	spdlog::debug("hello the game!");
	World world(kWindowWidth, kWindowHeight);
	world.run();

	return 0;
}