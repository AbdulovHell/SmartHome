#include "sys_headers.h"

#include "threading.h"
#include "Task.h"
#include "main.h"
#include "stuff.h"
#include "Colorize.h"
#include "DHT22.h"
#include "LightSensor.h"
#include "SoundSensor.h"
#include "SQLTX.h"
#include "MovingSensor.h"

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
	SoundSensor::SSenseInit(18);
	MovingSensor::MSenseInit(27);

	while (1) {
		if (cnt % 6 == 0) {	//1 minute interval			
			//Light
			float lightlvl = lt.GetLvl();
			WriteLog("TIMING", "Light lvl: " + ToStr(lightlvl), Stuff::Pink);
			//Temperature & Humiduty
			int attempts = 0;
			while(!DHT22::Read() && attempts++<5)
			{
				this_thread::sleep_for(std::chrono::milliseconds(20));
			}
			if (attempts<5) {
				float temp = DHT22::GetTemperature();
				float hum = DHT22::GetHumidity();
				WriteLog("TIMING", "Temperature: " + ToStr(temp), Stuff::Pink);
				WriteLog("TIMING", "Humiduty: " + ToStr(hum), Stuff::Pink);

				SendInfo(make_unique<MeasInfo>(temp,hum,lightlvl,0));
			}
			else {
				WriteLog("TIMING", "Error read() int DHT22.", Stuff::Pink);
			}
		}
		if (cnt % 24 == 0) {	//4 min
			SendInfo(make_unique<SoundInfo>(SoundSensor::GetState()));			
			SendInfo(make_unique<MovingInfo>(MovingSensor::GetState()));
			cnt = 0;
		}
		this_thread::sleep_for(std::chrono::seconds(10));
		++cnt;
	}
}

Threading::TimingThread::~TimingThread()
{
	pthread_cancel(threadHandle);
}
