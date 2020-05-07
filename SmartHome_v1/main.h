#ifndef _MAIN_H_
#define _MAIN_H_

#include <vector>
#include <mutex>

#include "Task.h"
#include "Colorize.h"

void InitShutdown();

namespace Threading {		
	void WriteLog(std::string src,std::string msg,Stuff::SColor clr);
}
#endif //_MAIN_H_