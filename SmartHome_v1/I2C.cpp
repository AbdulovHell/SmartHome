#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <iostream>
#include <unistd.h>

#include "I2C.h"

IO::I2C::I2C(int addr)
{
	fd = wiringPiI2CSetup(addr);
	this->addr = addr;
	if (fd < 0) {
		error_str = "I2C Open error";
	}
	else {
		opened = true;
	}
}

int IO::I2C::Read()
{
	return wiringPiI2CRead(fd);
}

short IO::I2C::Read2BytesFromReg(int reg)
{
	return (short)wiringPiI2CReadReg16(fd,reg);
}

uint8_t IO::I2C::ReadByteFromReg(int reg)
{
	return (uint8_t)wiringPiI2CReadReg8(fd, reg);
}

int IO::I2C::Write(uint8_t dt)
{
	return wiringPiI2CWrite(fd, dt);
}

size_t IO::I2C::Write(uint8_t * dt, size_t size)
{
	return write(fd, dt, 6);
}

int IO::I2C::WriteReg(uint8_t dt, int reg)
{
	return wiringPiI2CWriteReg8(fd, reg, dt);
}

int IO::I2C::WriteReg(uint16_t dt, int reg)
{
	return wiringPiI2CWriteReg16(fd, reg, dt);
}
