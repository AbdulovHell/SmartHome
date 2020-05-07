#ifndef _BMP085_H_
#define _BMP085_H_

#include "I2C.h"

class BMP085 {

	int address = 0x77;
	
	IO::I2C* bmp085;	

	short _AC1=408, _AC2=-72, _AC3=-14383;
	unsigned short _AC4=32741, _AC5=32757, _AC6=23153;
	short _B1=6190, _B2=4, _MB=-32768, _MC=-8711, _MD=2868;

	short _oss = 0;

	long _UT = 27898;
	long _UP = 23843;

	double Temperature = 500.0;
	double Pressure = -333.0;

	bool initialized = false;

	bool ReadCalibrationData();

	void ReadTemp_u();

	void ReadPressure_u();

    void Calc();

public:
	enum PressureType {
		Pa,
		kPa,
		hPa,
		mmHg
	};

	enum TemperatureType {
		C,
		K,
		F
	};

	BMP085(short oversampling_setting);

	void Initialize();
	
	bool IsInitialized();

	void ProceedMeasures();

	double GetTemperature(TemperatureType type);
	double GetPressure(PressureType type);	
	double GetAltitude();
};

#endif //#ifndef _BMP085_H_