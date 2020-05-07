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

	void React(int gpio, int level, uint32_t tick)
	{
		if (gpio == _pin && level!=2) {
			SendInfo(make_unique<SoundInfo>(1));
		}
	}

	void SSenseInit(int pin)
	{
		_pin = pin;
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