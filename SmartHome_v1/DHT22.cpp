#include "sys_headers.h"
#include "DHT22.h"
#include "main.h"
#include "stuff.h"

using namespace Threading;
using namespace Stuff;
using namespace std;

namespace DHT22 {

	int _pin;	
	uint8_t data[5];
	uint32_t tick_start;
	int waits;

	unsigned int _cycles[100];
	int _cycles_count;

	void aFunction(int gpio, int level, uint32_t tick)
	{
		if (gpio == _pin) {
			if (_cycles_count >= 100) {
				WriteLog("DHT", "Cycles overflow!", Stuff::Yellow);
				return;
			}
			_cycles[_cycles_count++] = tick;
		}
	}

	void DHT22Init(int pin) {
		_pin = pin;
		gpioSetMode(pin, PI_INPUT);  // Set GPIO17 as input.
		gpioSetPullUpDown(pin, PI_PUD_UP);   // Sets a pull-up.
	}

	bool Read() {
		//À
		memset(data, 0, 5);
		_cycles_count = 0;
		memset(_cycles, 0, sizeof(uint32_t) * 100);
		waits = 0;
		//Á
		gpioSetMode(_pin, PI_OUTPUT);  // Set GPIO17 as input.
		gpioWrite(_pin, 0); // Set GPIO17 low.
		this_thread::sleep_for(chrono::milliseconds(2));

		gpioSetMode(_pin, PI_INPUT);  // Set GPIO17 as input.
		gpioSetPullUpDown(_pin, PI_PUD_UP);   // Sets a pull-up.
		//Â
		this_thread::sleep_for(chrono::microseconds(35));
		//Ã-Ç
		
		tick_start = gpioTick();
		gpioSetAlertFunc(_pin, aFunction);

		while (_cycles_count <= 83) {
			this_thread::sleep_for(chrono::milliseconds(2));
			waits++;
			if (waits >= 100)
				break;
		}

		//WriteLog("DHT", "Data recived. Pulses: " + ToStr(_cycles_count), Stuff::Yellow);
		//WriteLog("DHT", "Turn off alert", Stuff::Yellow);

		gpioSetAlertFunc(_pin, NULL);

		for (int i = _cycles_count - 1; i >= 1; i--) {
			_cycles[i] = _cycles[i] - _cycles[i - 1];
		}
		_cycles[0] = _cycles[0] - tick_start;

		/*for (int i = 0; i < _cycles_count; i++) {
			WriteLog("DHT", "c[" + ToStr(i) + "]=" + ToStr(_cycles[i]), Stuff::Yellow);
		}*/

		uint32_t pairs[80];
		int offset = _cycles_count - (1 + 80);
		//WriteLog("DHT", "Offset: "+ToStr(offset), Stuff::Yellow);
		memcpy(pairs,(_cycles+offset), sizeof(uint32_t) * 80);
		//WriteLog("DHT","Last two cycles: "+ToStr(_cycles[_cycles_count-2])+" "+ToStr(_cycles[_cycles_count - 1]),Stuff::Yellow);

		for (int i = 0; i < 40; i++) {
			uint32_t clk = pairs[2 * i];
			uint32_t dt = pairs[2 * i + 1];

			/*char buf[100];
			sprintf(buf,"clk: %d dt: %d",clk,dt);
			WriteLog("DHT", buf, Stuff::Yellow);*/

			data[i / 8] <<= 1;
			data[i / 8] |= clk > dt ? 0 : 1;			
		}
		
		// Checksum
		if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
			WriteLog("DHT", "Read() completed", Stuff::Yellow);
			return true;
		}
		else {
			WriteLog("DHT", "Checksum error", Stuff::Yellow);
			return false;
		}
		//À
	}

	float GetTemperature()
	{
		int temp = ((data[2]<<8) + data[3])&0x7FFF;
		return (temp/10.0)*(data[2]&0x80?-1:1);
	}

	float GetHumidity()
	{
		int hum = (data[0] << 8) + data[1];
		return hum/10.0;
	}

}
