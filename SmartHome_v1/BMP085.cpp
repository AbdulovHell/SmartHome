#include "sys_headers.h"
#include "BMP085.h"
#include "main.h"
#include "stuff.h"
#include <cmath>

using namespace std;
using namespace Stuff;

BMP085::BMP085(short oversampling_setting)
{
	if (oversampling_setting > 3) oversampling_setting = 3;
	if (oversampling_setting < 0) oversampling_setting = 0;
	_oss = oversampling_setting;
}

void BMP085::Initialize()
{
	bmp085 = new IO::I2C(address);
	initialized = ReadCalibrationData();
}

bool BMP085::ReadCalibrationData()
{
	_AC1 = bmp085->Read2BytesFromReg(0xAA + 0);
	Threading::WriteLog("BMP085","AC1: "+ ToStr(_AC1),Stuff::LightBlue);
	_AC2 = bmp085->Read2BytesFromReg(0xAA + 2);
	Threading::WriteLog("BMP085", "AC2: " + ToStr(_AC2), Stuff::LightBlue);
	_AC3 = bmp085->Read2BytesFromReg(0xAA + 4);
	Threading::WriteLog("BMP085", "AC3: " + ToStr(_AC3), Stuff::LightBlue);
	_AC4 = bmp085->Read2BytesFromReg(0xAA + 6);
	Threading::WriteLog("BMP085", "AC4: " + ToStr(_AC4), Stuff::LightBlue);
	_AC5 = bmp085->Read2BytesFromReg(0xAA + 8);
	Threading::WriteLog("BMP085", "AC5: " + ToStr(_AC5), Stuff::LightBlue);
	_AC6 = bmp085->Read2BytesFromReg(0xAA + 10);
	Threading::WriteLog("BMP085", "AC6: " + ToStr(_AC6), Stuff::LightBlue);
	_B1 = bmp085->Read2BytesFromReg(0xAA + 12);
	Threading::WriteLog("BMP085", "B1: " + ToStr(_B1), Stuff::LightBlue);
	_B2 = bmp085->Read2BytesFromReg(0xAA + 14);
	Threading::WriteLog("BMP085", "B2: " + ToStr(_B2), Stuff::LightBlue);
	_MB = bmp085->Read2BytesFromReg(0xAA + 16);
	Threading::WriteLog("BMP085", "MB: " + ToStr(_MB), Stuff::LightBlue);
	_MC = bmp085->Read2BytesFromReg(0xAA + 18);
	Threading::WriteLog("BMP085", "MC: " + ToStr(_MC), Stuff::LightBlue);
	_MD = bmp085->Read2BytesFromReg(0xAA + 20);
	Threading::WriteLog("BMP085", "MD: " + ToStr(_MD), Stuff::LightBlue);
	return true;
}

void BMP085::ReadTemp_u()
{
	bmp085->WriteReg((uint8_t)0x2E, (uint8_t)0xF4);

	delay(100);
	
	_UT = bmp085->Read2BytesFromReg(0xF6);
	Threading::WriteLog("BMP085", "UT: " + ToStr(_UT), Stuff::LightBlue);	
}

void BMP085::ReadPressure_u()
{
	bmp085->WriteReg((uint8_t)(0x34 + (_oss << 6)), (uint8_t)0xF4);

	delay(300);
	
	long temp_arr[3] = { 0, };

	for (int i = 0; i < 3; i++) {
		temp_arr[i] = bmp085->ReadByteFromReg(0xF6 + i);
	}

	_UP = ((temp_arr[0] << 16) + (temp_arr[1] << 8) + temp_arr[2]) >> (8 - _oss);
	Threading::WriteLog("BMP085", "UP: " + ToStr(_UP), Stuff::LightBlue);
}

void BMP085::Calc()
{
	double X1 = (_UT - _AC6)*_AC5 / pow(2, 15);
	double X2 = _MC * pow(2, 11) / (X1 + _MD);
	double B5 = X1 + X2;
	double T = (B5 + 8) / pow(2, 4); //0.1 C
	Temperature = T * 0.1;	//C

	double B6 = B5 - 4000;
	X1 = (_B2*(B6*B6 / pow(2, 12))) / pow(2, 11);
	//Serial.println("\nX1: "); Serial.print(X1);
	X2 = _AC2 * B6 / pow(2, 11);
	//Serial.println("\nX2: "); Serial.print(X2);
	double X3 = X1 + X2;
	double B3 = ((((long)_AC1 * 4 + (long)X3) << _oss) + 2) / 4;
	//Serial.println("\nB3: "); Serial.print(B3);
	X1 = _AC3 * B6 / pow(2, 13);
	X2 = (_B1*(B6*B6 / pow(2, 12))) / pow(2, 16);
	X3 = ((X1 + X2) + 2) / pow(2, 2);
	double B4 = _AC4 * (X3 + 32768) / pow(2, 15);
	double B7 = (_UP - B3)*(50000 >> _oss);
	double p = 0;
	//Serial.println("\nB7: "); Serial.print(B7);
	if (B7 < 2147483648.0) {
		p = (B7 * 2) / B4;
	}
	else {
		p = (B7 / B4) * 2;
	}
	X1 = (p / pow(2, 8))*(p / pow(2, 8));
	X1 = (X1 * 3038) / pow(2, 16);
	X2 = (-7357 * p) / pow(2, 16);
	p = p + (X1 + X2 + 3791) / pow(2, 4); //Pa
	Pressure = p;
}

bool BMP085::IsInitialized()
{
	return initialized;
}

void BMP085::ProceedMeasures()
{
	ReadTemp_u();
	ReadPressure_u();
	Calc();
}

double BMP085::GetTemperature(TemperatureType type)
{
	switch (type)
	{
	case BMP085::C:
		return Temperature;
		break;
	case BMP085::K:
		return Temperature + 273.15;
		break;
	case BMP085::F:
		return Temperature * 1.8 + 32.0;
		break;
	default:
		return Temperature;
		break;
	}
}

double BMP085::GetPressure(PressureType type)
{
	switch (type)
	{
	case BMP085::Pa:
		return Pressure;
		break;
	case BMP085::kPa:
		return Pressure / 1000.0;
		break;
	case BMP085::hPa:
		return Pressure * 0.01;
		break;
	case BMP085::mmHg:
		return (Pressure / 1000.0)*7.5;
		break;
	default:
		return Pressure;
		break;
	}
}

double BMP085::GetAltitude()
{
	double alt = 44330 * (1 - pow((Pressure*0.01) / 1013.25, 1 / 5.255));
	return alt;
}
