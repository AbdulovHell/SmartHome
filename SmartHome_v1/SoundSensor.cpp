#include "sys_headers.h"
#include "SoundSensor.h"
#include "main.h"
#include "stuff.h"
#include "SQLTX.h"

using namespace Threading;
using namespace Stuff;
using namespace std;

namespace SoundSensor {

	int _pin;
	
	uint32_t f_ticks;
	const uint32_t pause = 3 * 1000 * 1000; //3s	

	void React(int gpio, int level, uint32_t tick)
	{
		if (gpio == _pin && level==1) {
			WriteLog("SS", "Act", Stuff::LightGray);
			if (tick - f_ticks > pause) {
				WriteLog("SS", "Send", Stuff::LightGray);
				SendInfo(make_unique<SoundInfo>(1));
				f_ticks = tick;
			}			
		}
	}

	void SSenseInit(int pin)
	{
		_pin = pin;
		gpioSetMode(_pin, PI_INPUT);
		gpioSetPullUpDown(_pin, PI_PUD_OFF);
		this_thread::sleep_for(chrono::milliseconds(20));
		f_ticks = gpioTick();
		gpioSetAlertFunc(_pin, React);		
	}

	int GetState()
	{
		gpioSetAlertFunc(_pin, NULL);
		int state=gpioRead(_pin);
		gpioSetAlertFunc(_pin, React);
		return state;
	}
}