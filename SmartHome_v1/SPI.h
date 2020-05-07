#ifndef _SPI_H_
#define _SPI_H_

namespace IO {

	class SPI {
		int fd = -1;
		int ch = -1;
		std::string error_str;
		bool opened = false;
	public:
		SPI(int ch, int spd);
		SPI(int ch, int spd, int mode);
		bool DataRW(uint8_t* RWBuf, size_t size);
	};
}
#endif	// _SPI_H_
