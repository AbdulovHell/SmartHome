#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <iostream>

#include "SPI.h"

IO::SPI::SPI(int ch, int spd)
{
	fd = wiringPiSPISetup(ch, spd);
	this->ch = ch;
	if (fd < 0) {
		error_str = "SPI Open error";
	}
	else {
		opened = true;
	}
}

IO::SPI::SPI(int ch, int spd, int mode)
{
	fd = wiringPiSPISetupMode(ch, spd, mode);
	this->ch = ch;
	if (fd < 0) {
		error_str = "SPI Open error";
	}
	else {
		opened = true;
	}
}

bool IO::SPI::DataRW(uint8_t * RWBuf, size_t size)
{
	wiringPiSPIDataRW(ch, RWBuf, size);
	uint32_t sym = 0;
	for (size_t i = 0; i < size; i++)
		sym += RWBuf[i];
	if (sym > 0)
		return true;
	else
		return false;
}
