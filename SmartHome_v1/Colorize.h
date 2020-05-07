#ifndef _COLORIZE_H_
#define _COLORIZE_H_
#include <iostream>

//инструмент раскраски вывода в терминал

namespace Stuff {
	typedef const std::string SColor;

	extern SColor __reset;

	extern SColor Red;
	extern SColor Green;
	extern SColor Yellow;
	extern SColor Blue;
	extern SColor Pink;
	extern SColor LightBlue;
	extern SColor LightGray;

	std::string MakeColor(std::string src, SColor clr);
}

#endif	// _COLORIZE_H_
