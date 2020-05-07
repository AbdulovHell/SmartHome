#ifndef _DHT22_H_
#define _DHT22_H_

namespace DHT22 {
		
		extern int _pin;
		extern uint8_t data[5];
		extern uint32_t tick_start;
		extern int waits;

		extern unsigned int _cycles[100];
		extern int _cycles_count;

		void aFunction(int gpio, int level, uint32_t tick);

		void DHT22Init(int pin);

		bool Read();

		float GetTemperature();

		float GetHumidity();
}

#endif
