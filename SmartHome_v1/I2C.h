#ifndef _I2C_H_
#define _I2C_H_

namespace IO {
	//I2C support library
	class I2C {
		int fd = -1;
		bool opened = false;
		std::string error_str;
		int addr = 0;

	public:
		//адрес устройства для подключения
		I2C(int addr);


		//чтение байта с у-ва
		int Read();

		short Read2BytesFromReg(int reg);
		uint8_t ReadByteFromReg(int reg);

		//запись байта в у-во
		int Write(uint8_t dt);
		//запись массива байт в у-во
		size_t Write(uint8_t* dt, size_t size);
		//запись байта в регистр у-ва
		int WriteReg(uint8_t dt, int reg);
		//запись двух байт в регистр у-ва
		int WriteReg(uint16_t dt, int reg);

		bool IsOpen() {
			return opened;
		}

		std::string GetError() {
			return error_str;
		}
		//адрес подключенного у-ва
		int Addr() { return addr; }
		int FileDesc() { return fd; }
	};
}
#endif	// _I2C_H_
