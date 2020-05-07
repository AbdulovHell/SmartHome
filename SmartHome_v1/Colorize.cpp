#include "Colorize.h"

namespace Stuff {
	SColor __reset = "\033[0m";

	SColor Red = "\033[0;31m";
	SColor Green = "\033[0;32m";
	SColor Yellow = "\033[0;33m";
	SColor Blue = "\033[0;34m";
	SColor Pink = "\033[0;35m";
	SColor LightBlue = "\033[0;36m";
	SColor LightGray = "\033[0;37m";
}

std::string Stuff::MakeColor(std::string src, SColor clr)
{
	std::string res = clr + src + __reset;
	return res;
}
