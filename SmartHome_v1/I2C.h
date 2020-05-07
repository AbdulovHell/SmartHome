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
		//����� ���������� ��� �����������
		I2C(int addr);


		//������ ����� � �-��
		int Read();

		short Read2BytesFromReg(int reg);
		uint8_t ReadByteFromReg(int reg);

		//������ ����� � �-��
		int Write(uint8_t dt);
		//������ ������� ���� � �-��
		size_t Write(uint8_t* dt, size_t size);
		//������ ����� � ������� �-��
		int WriteReg(uint8_t dt, int reg);
		//������ ���� ���� � ������� �-��
		int WriteReg(uint16_t dt, int reg);

		bool IsOpen() {
			return opened;
		}

		std::string GetError() {
			return error_str;
		}
		//����� ������������� �-��
		int Addr() { return addr; }
		int FileDesc() { return fd; }
	};
}
#endif	// _I2C_H_
