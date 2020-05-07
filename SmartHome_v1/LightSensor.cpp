#include "sys_headers.h"
#include "LightSensor.h"

void LightSensor::Read()
{
	digitalWrite(CS, 0);
	dev->DataRW(data, 2);
	digitalWrite(CS, 1);
}

LightSensor::LightSensor(int cs)
{
	CS = cs;
	pinMode(CS, OUTPUT);
	digitalWrite(CS, 1);
	dev = new IO::SPI(0, 10000);
}

LightSensor::~LightSensor()
{
	delete dev;
}

float LightSensor::GetLvl()
{
	Read();
	int adc_val = ((data[0] << 8) + data[1]) >> 1;
	return ((4095 - adc_val) / 4095.0)*100.0;
}
