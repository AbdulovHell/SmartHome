#include "sys_headers.h"
#include "MovingSensor.h"
#include "main.h"
#include "stuff.h"
#include "SQLTX.h"

using namespace Threading;
using namespace Stuff;
using namespace std;

namespace MovingSensor {

	int _pin;

	void React(int gpio, int level, uint32_t tick)
	{
		if (gpio == _pin && level == 1) {
			WriteLog("MS","Act",Stuff::Blue);
			SendInfo(make_unique<MovingInfo>(1));
		}
	}

	void MSenseInit(int pin)
	{
		_pin = pin;
		gpioSetMode(_pin, PI_INPUT);
		gpioSetPullUpDown(_pin, PI_PUD_DOWN);
		this_thread::sleep_for(chrono::milliseconds(20));
		gpioSetAlertFunc(_pin, React);
	}

	int GetState()
	{
		gpioSetAlertFunc(_pin, NULL);
		int state = gpioRead(_pin);
		gpioSetAlertFunc(_pin, React);
		return state;
	}
}
