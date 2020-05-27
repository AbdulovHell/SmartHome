#include "sys_headers.h"
#include "threading.h"
#include "Colorize.h"
#include "stuff.h"
#include "main.h"
//#include "BMP085.h"
#include "SQLTX.h"

using namespace Threading;
using namespace Stuff;

//TODO: ƒатчик звука, настроили чувствительность и убрали повторные срабатывани€, посмотрим

namespace Threading {
	bool Working = true;
	TimingThread* timingThrd;
	SQLTXThread* sqltxThrd;
	
	void WriteLog(string src, string msg, Stuff::SColor clr)
	{
		char buf[128];
		memset(buf, 0, 128);

		sprintf(buf, "%s: %s", Stuff::MakeColor(src, clr).c_str(),msg.c_str());
		cout << buf << endl;
	}
}

void InitShutdown() {
	Working = false;
	timingThrd->~TimingThread();
	sqltxThrd->~SQLTXThread();
	
	WriteLog("MAIN","Threads stopped", Stuff::Green);
}

int main(int argc, char* argv[])
{
	wiringPiSetupGpio();

	//gpioTerminate();

	gpioCfgSetInternals(1 << 10);
	gpioCfgClock(2, 1, NULL);

	if (gpioInitialise() < 0)
	{
		WriteLog("MAIN", "gpioInitialise failed", Stuff::Green);
	}

	sqltxThrd = new SQLTXThread();
	timingThrd = new TimingThread();

	/*BMP085* det = new BMP085(0);
	det->Initialize();
	det->ProceedMeasures();
	WriteLog("MAIN", "Temperature: "+ToStr(det->GetTemperature(BMP085::TemperatureType::C)), Stuff::Green);
	WriteLog("MAIN", "Pressure: " + ToStr(det->GetPressure(BMP085::PressureType::mmHg)), Stuff::Green);*/	
	
	while (Working)
	{
		this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	return 0;
}