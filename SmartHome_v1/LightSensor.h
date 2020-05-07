#ifndef _LIGHTSENSOR_H_
#define _LIGHTSENSOR_H_

#include "SPI.h"

class LightSensor {
	IO::SPI* dev;
	int CS;
	uint8_t data[2];

	void Read();

public:
	LightSensor(int cs);

	~LightSensor();

	float GetLvl();
};	
#endif
