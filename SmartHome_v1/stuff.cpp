#include "sys_headers.h"

#include "stuff.h"

namespace Stuff {
	std::string ToStr(short num)
	{
		char buf[16];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(int num)
	{
		char buf[16];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(uint32_t num)
	{
		char buf[16];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(long num)
	{
		char buf[32];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(uint64_t num)
	{
		char buf[32];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(int64_t num)
	{
		char buf[32];
		sprintf(buf, "%d", num);
		return std::string(buf);
	}

	std::string ToStr(double num)
	{
		char buf[32];
		sprintf(buf, "%.2f", num);
		return std::string(buf);
	}
}

bool Stuff::Verify(char * buf)
{
	if (buf[0] != 'e')  return false;	//базовая проверка
	size_t len = (buf[2] + (buf[3] << 8));	//получаем полезную длинну сообщения
	uint8_t mCK_A = buf[3 + len + 1], mCK_B = buf[3 + len + 2];	//суммы, поставляемые клиентом
	uint8_t cCK_A = 0, cCK_B = 0;	//наши суммы
	for (size_t i = 1; i < (3 + len + 1); i++) {//считаем
		cCK_A = (uint8_t)(cCK_A + (uint8_t)buf[i]);
		cCK_B = (uint8_t)(cCK_B + cCK_A);
	}
	if ((cCK_A == mCK_A) && (cCK_B == mCK_B)) //сравниваем
		return true;	//совпали
	return false;	//облом
}

void Stuff::CalcSum(char * buf, size_t size)
{
	uint8_t CK_A = 0, CK_B = 0;
	for (size_t i = 1; i < size - 2; i++) {
		CK_A = (uint8_t)(CK_A + (uint8_t)buf[i]);
		CK_B = (uint8_t)(CK_B + CK_A);
	}
	buf[size - 1] = CK_B;
	buf[size - 2] = CK_A;
}

int Stuff::RangeRand(int min, int max)
{
	max++;
	int rnd = rand();
	//printf("RR %d	",rnd);
	return (int)((double)rnd / ((double)RAND_MAX + 1.0)*(max - min) + min);
}


