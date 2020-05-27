#ifndef _BMP280_H_
#define _BMP280_H_

#include "I2C.h"

typedef int BMP280_S32_t;
typedef uint BMP280_U32_t;
typedef long BMP280_S64_t;
typedef ulong BMP280_U64_t;

class BMP280 {

	int address = 0x76;
	
	IO::I2C* bmp280;	

	unsigned short dig_T1=27504, dig_P1=36477;
	short dig_T2=26435, dig_T3=-1000;
	short dig_P2=-10685, dig_P3=3024, dig_P4=2855, dig_P5=140, dig_P6=-7, dig_P7=15500, dig_P8=-14600, dig_P9=6000;

	//short _oss = 0;
	uint8_t meas_start = 0b01011101; //T x2, P x16, Forced mode
	
	BMP280_S32_t t_fine;

	long _UT = 519888;
	long _UP = 415148;

	double Temperature = 500.0;
	double Pressure = -333.0;

	bool initialized = false;

	bool ReadCalibrationData();

	void Read();

	BMP280_S32_t Compensate_T(BMP280_S32_t adc_T);
	double Compensate_P(BMP280_S32_t adc_P);

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

	BMP280(/*short oversampling_setting*/);

	void Initialize();
	
	bool IsInitialized();

	void ProceedMeasures();

	double GetTemperature(TemperatureType type);
	double GetPressure(PressureType type);	
	double GetAltitude();
};

#endif //#ifndef _BMP280_H_