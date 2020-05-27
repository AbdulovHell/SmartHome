#include "sys_headers.h"
#include "BMP280.h"
#include "main.h"
#include "stuff.h"
#include <cmath>

using namespace std;
using namespace Stuff;

BMP280::BMP280(/*short oversampling_setting*/)
{
	//if (oversampling_setting > 3) oversampling_setting = 3;
	//if (oversampling_setting < 0) oversampling_setting = 0;
	//_oss = oversampling_setting;
}

void BMP280::Initialize()
{
	bmp280 = new IO::I2C(address);	
		
	//Debug
	/*{
		double t = Compensate_T(_UT)*0.01;
		double p = Compensate_P(_UP);
		Threading::WriteLog("BMP280", "init: " + ToStr(t)+" "+ToStr(p), Stuff::LightBlue);
	}*/

	uint8_t confreg = 0b00001000;	//filter = 2 -> coef = 4
	bmp280->WriteReg(confreg, 0xF5);	

	initialized = ReadCalibrationData();	
}

bool BMP280::ReadCalibrationData()
{
	int staAddr = 0x88;

	//T
	dig_T1 = bmp280->Read2BytesFromReg(staAddr + 0);
	//Threading::WriteLog("BMP280", "dig_T1: " + ToStr(dig_T1), Stuff::LightBlue);
	dig_T2 = bmp280->Read2BytesFromReg(staAddr + 2);
	//Threading::WriteLog("BMP280", "dig_T2: " + ToStr(dig_T2), Stuff::LightBlue);
	dig_T3 = bmp280->Read2BytesFromReg(staAddr + 4);
	//Threading::WriteLog("BMP280", "dig_T3: " + ToStr(dig_T3), Stuff::LightBlue);

	//P
	dig_P1 = bmp280->Read2BytesFromReg(staAddr + 6);
	//Threading::WriteLog("BMP280", "dig_P1: " + ToStr(dig_P1), Stuff::LightBlue);
	dig_P2 = bmp280->Read2BytesFromReg(staAddr + 8);
	//Threading::WriteLog("BMP280", "dig_P2: " + ToStr(dig_P2), Stuff::LightBlue);
	dig_P3 = bmp280->Read2BytesFromReg(staAddr + 10);
	//Threading::WriteLog("BMP280", "dig_P3: " + ToStr(dig_P3), Stuff::LightBlue);
	dig_P4 = bmp280->Read2BytesFromReg(staAddr + 12);
	//Threading::WriteLog("BMP280", "dig_P4: " + ToStr(dig_P4), Stuff::LightBlue);
	dig_P5 = bmp280->Read2BytesFromReg(staAddr + 14);
	//Threading::WriteLog("BMP280", "dig_P5: " + ToStr(dig_P5), Stuff::LightBlue);
	dig_P6 = bmp280->Read2BytesFromReg(staAddr + 16);
	//Threading::WriteLog("BMP280", "dig_P6: " + ToStr(dig_P6), Stuff::LightBlue);
	dig_P7 = bmp280->Read2BytesFromReg(staAddr + 18);
	//Threading::WriteLog("BMP280", "dig_P7: " + ToStr(dig_P7), Stuff::LightBlue);
	dig_P8 = bmp280->Read2BytesFromReg(staAddr + 20);
	//Threading::WriteLog("BMP280", "dig_P8: " + ToStr(dig_P8), Stuff::LightBlue);
	dig_P9 = bmp280->Read2BytesFromReg(staAddr + 22);
	//Threading::WriteLog("BMP280", "dig_P9: " + ToStr(dig_P9), Stuff::LightBlue);
	return true;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123”equals 51.23 DegC. 
// t_fine carries fine temperature as global value

BMP280_S32_t BMP280::Compensate_T(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1  = ((((adc_T>>3) -((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2  = (((((adc_T>>4) -((BMP280_S32_t)dig_T1)) * ((adc_T>>4) -((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T  = (t_fine * 5 + 128) >> 8;
	return T;
}
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8fractional bits).
// Output value of “24674867”represents 24674867/256 = 96386.2 Pa = 963.862 hPa
double BMP280::Compensate_P(BMP280_S32_t adc_P)
{
	double var1, var2, p;
	var1 = ((double)t_fine/2.0) - 64000.0;
	var2 = var1 * var1 * ((double)dig_P6)/32768.0;
	var2 = var2 + var1*(double)dig_P5*2.0;
	var2 = var2/4.0 + ((double)dig_P4)*65536.0;
	var1 = (((double)dig_P3)*var1*var1 / 524288.0 + ((double)dig_P2)*var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);	
	p = 1048576.0 - (double)adc_P;
	p = (p - (var2 / 4096.0))*6250.0 / var1;
	var1 = ((double)dig_P9)*p*p/2147483648.0;
	var2 = p*((double)dig_P8)/32768.0;
	p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
	return p;
}

void BMP280::Read()
{
	int temp_arr[3] = { 0, };
	int pres_arr[3] = { 0, };

	for (int i = 0; i < 3; i++) {
		temp_arr[i] = bmp280->ReadByteFromReg(0xFA + i);
	}
	for (int i = 0; i < 3; i++) {
		pres_arr[i] = bmp280->ReadByteFromReg(0xF7 + i);
	}

	_UT = ((temp_arr[0] << 16) + (temp_arr[1] << 8) + temp_arr[2]) >> 4;
	_UP = ((pres_arr[0] << 16) + (pres_arr[1] << 8) + pres_arr[2]) >> 4;
	//Threading::WriteLog("BMP280", "UT: " + ToStr(_UT), Stuff::LightBlue);
	//Threading::WriteLog("BMP280", "UP: " + ToStr(_UP), Stuff::LightBlue);
}

bool BMP280::IsInitialized()
{
	return initialized;
}

void BMP280::ProceedMeasures()
{
	bmp280->WriteReg(meas_start, 0xF4);
	this_thread::sleep_for(chrono::milliseconds(60));
	Read();
	Temperature=Compensate_T(_UT)*0.01;
	Pressure=Compensate_P(_UP);
	//Calc();
}

double BMP280::GetTemperature(TemperatureType type)
{
	switch (type)
	{
	case BMP280::C:
		return Temperature;
		break;
	case BMP280::K:
		return Temperature + 273.15;
		break;
	case BMP280::F:
		return Temperature * 1.8 + 32.0;
		break;
	default:
		return Temperature;
		break;
	}
}

double BMP280::GetPressure(PressureType type)
{
	switch (type)
	{
	case BMP280::Pa:
		return Pressure;
		break;
	case BMP280::kPa:
		return Pressure / 1000.0;
		break;
	case BMP280::hPa:
		return Pressure * 0.01;
		break;
	case BMP280::mmHg:
		return (Pressure / 1000.0)*7.5;
		break;
	default:
		return Pressure;
		break;
	}
}

double BMP280::GetAltitude()
{
	double alt = 44330 * (1 - pow((Pressure*0.01) / 1013.25, 1 / 5.255));
	return alt;
}
