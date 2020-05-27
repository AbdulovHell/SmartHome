#include "sys_headers.h"

#include "threading.h"
#include "main.h"
#include "stuff.h"
#include "Colorize.h"
#include "DHT22.h"
#include "LightSensor.h"
#include "SoundSensor.h"
#include "SQLTX.h"
#include "MovingSensor.h"
#include "BMP280.h"

using namespace Stuff;

namespace Threading {
	//bool UpdateNow;
}

Threading::Thread::Thread(void*(func)(void*), void* arg)
{
	pthread_create(&threadHandle, NULL, func, arg);
}

Threading::Thread::~Thread()
{
	pthread_cancel(threadHandle);
}

pthread_t Threading::Thread::GetThrdHandle()
{
	return threadHandle;
}

int Threading::Thread::Join()
{
	return pthread_join(threadHandle, NULL);
}

void * Threading::TimingThread::Timing(void * ptr_null)
{
	WriteLog("TIMING", "Start timing", Stuff::Pink);
	int cnt = 0;

	DHT22::DHT22Init(17);
	LightSensor lt(22);
	SoundSensor::SSenseInit(23);
	MovingSensor::MSenseInit(27);
	BMP280 bmp280;
	bmp280.Initialize();

	while (1) {
		if (cnt % 15 == 0) {	//15 minutes interval	
			//Pressure and temprepature
			bmp280.ProceedMeasures();
			double press = bmp280.GetPressure(BMP280::mmHg);
			double tmpr = bmp280.GetTemperature(BMP280::C);
			WriteLog("TIMING", "Pressure: " + ToStr(press) + " mmHg", Stuff::Pink);
			WriteLog("TIMING", "Temperature: " + ToStr(tmpr) + " C", Stuff::Pink);
			//Light
			float lightlvl = lt.GetLvl();
			WriteLog("TIMING", "Light lvl: " + ToStr(lightlvl), Stuff::Pink);
			//Temperature & Humiduty
			int attempts = 0;
			while (!DHT22::Read() && attempts++ < 3)
			{
				this_thread::sleep_for(std::chrono::milliseconds(120));
			}
			if (attempts < 5) {
				//float temp = DHT22::GetTemperature();
				float hum = DHT22::GetHumidity();
				//WriteLog("TIMING", "Temperature: " + ToStr(temp), Stuff::Pink);
				WriteLog("TIMING", "Humiduty: " + ToStr(hum) + " %", Stuff::Pink);
				if (hum >= 0 && hum <= 100.0)
					SendInfo(make_unique<MeasInfo>(tmpr, hum, lightlvl, press));
				
			}
			else {
				WriteLog("TIMING", "Error read() int DHT22.", Stuff::Pink);
			}
		}
		if (cnt % 60 == 0) {	//60 min
			SendInfo(make_unique<SoundInfo>(SoundSensor::GetState()));
			SendInfo(make_unique<MovingInfo>(MovingSensor::GetState()));
			cnt = 0;
		}
		this_thread::sleep_for(std::chrono::minutes(1));
		++cnt;
	}
}

Threading::TimingThread::~TimingThread()
{
	pthread_cancel(threadHandle);
}
